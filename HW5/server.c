#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>

#define MAX 1024
#define ACCOUNT_FILE "account.txt"

typedef struct {
    char username[50];
    char password[50];
    char email[100];
    char homepage[100];
    int status;
} Account;

int loadAccounts(Account accounts[], int *n) {
    FILE *f = fopen(ACCOUNT_FILE, "r");
    if (!f) return 0;
    *n = 0;
    while (fscanf(f, "%s %s %s %s %d",
                  accounts[*n].username,
                  accounts[*n].password,
                  accounts[*n].email,
                  accounts[*n].homepage,
                  &accounts[*n].status) == 5) {
        (*n)++;
    }
    fclose(f);
    return 1;
}

void saveAccounts(Account accounts[], int n) {
    FILE *f = fopen(ACCOUNT_FILE, "w");
    for (int i = 0; i < n; i++) {
        fprintf(f, "%s %s %s %s %d\n",
                accounts[i].username,
                accounts[i].password,
                accounts[i].email,
                accounts[i].homepage,
                accounts[i].status);
    }
    fclose(f);
}

int findAccount(Account accounts[], int n, char *username) {
    for (int i = 0; i < n; i++)
        if (strcmp(accounts[i].username, username) == 0)
            return i;
    return -1;
}

int validPassword(const char *pw) {
    for (int i = 0; i < strlen(pw); i++)
        if (!isalnum(pw[i]))
            return 0;
    return 1;
}

void splitPassword(const char *pw, char *letters, char *digits) {
    int li = 0, di = 0;
    for (int i = 0; i < strlen(pw); i++) {
        if (isalpha(pw[i])) letters[li++] = pw[i];
        else if (isdigit(pw[i])) digits[di++] = pw[i];
    }
    letters[li] = '\0';
    digits[di] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./server <PortNumber>\n");
        return 1;
    }

    int port = atoi(argv[1]);
    int server_sock, client_sock;
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);
    char buffer[MAX];
    Account accounts[100];
    int n = 0;

    if (!loadAccounts(accounts, &n)) {
        printf("Cannot open account file.\n");
        return 1;
    }

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    bind(server_sock, (struct sockaddr*)&server, sizeof(server));
    listen(server_sock, 5);
    printf("Server is listening on port %d...\n", port);

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client, &len);
        printf("Client connected.\n");

        int loginAttempts = 0;
        int loggedIn = 0;
        int idx = -1;

        recv(client_sock, buffer, MAX, 0);
        buffer[strcspn(buffer, "\n")] = '\0';
        idx = findAccount(accounts, n, buffer);

        if (idx == -1) {
            send(client_sock, "Account not found\n", 18, 0);
            close(client_sock);
            continue;
        }

        send(client_sock, "Insert password\n", 16, 0);
        while (loginAttempts < 3 && !loggedIn) {
            recv(client_sock, buffer, MAX, 0);
            buffer[strcspn(buffer, "\n")] = '\0';

            if (strcmp(buffer, accounts[idx].password) == 0) {
                if (accounts[idx].status == 1) {
                    send(client_sock, "OK\n", 3, 0);
                    loggedIn = 1;
                } else {
                    send(client_sock, "Account not ready\n", 18, 0);
                    break;
                }
            } else {
                loginAttempts++;
                if (loginAttempts == 3) {
                    accounts[idx].status = 0;
                    saveAccounts(accounts, n);
                    send(client_sock, "Account is blocked\n", 19, 0);
                    break;
                } else {
                    send(client_sock, "Not OK\n", 7, 0);
                }
            }
        }

        if (!loggedIn) {
            close(client_sock);
            continue;
        }

        while (1) {
            memset(buffer, 0, MAX);
            recv(client_sock, buffer, MAX, 0);
            buffer[strcspn(buffer, "\n")] = '\0';

            if (strcmp(buffer, "bye") == 0) {
                char msg[100];
                sprintf(msg, "Goodbye %s\n", accounts[idx].username);
                send(client_sock, msg, strlen(msg), 0);
                break;
            } else if (strcmp(buffer, "homepage") == 0) {
                send(client_sock, accounts[idx].homepage, strlen(accounts[idx].homepage), 0);
            } else {
                if (!validPassword(buffer)) {
                    send(client_sock, "Error\n", 6, 0);
                } else {
                    char letters[100], digits[100], result[300];
                    splitPassword(buffer, letters, digits);
                    snprintf(result, sizeof(result), "%s %s\n", letters, digits);
                    send(client_sock, result, strlen(result), 0);
                    strcpy(accounts[idx].password, buffer);
                    saveAccounts(accounts, n);
                }
            }
        }

        close(client_sock);
        printf("Client disconnected.\n");
    }

    close(server_sock);
    return 0;
}
