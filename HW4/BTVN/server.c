#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>

#define BUFF_SIZE 1024

typedef struct {
    char username[50];
    char password[50];
    char email[100];
    char homepage[100];
    int status; // 1: active, 0: blocked
} Account;

int loadAccounts(Account *accounts, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return 0;
    int i = 0;
    while (fscanf(f, "%s %s %s %s %d",
                  accounts[i].username,
                  accounts[i].password,
                  accounts[i].email,
                  accounts[i].homepage,
                  &accounts[i].status) == 5) {
        i++;
    }
    fclose(f);
    return i;
}

void saveAccounts(Account *accounts, int n, const char *filename) {
    FILE *f = fopen(filename, "w");
    for (int i = 0; i < n; i++)
        fprintf(f, "%s %s %s %s %d\n",
                accounts[i].username,
                accounts[i].password,
                accounts[i].email,
                accounts[i].homepage,
                accounts[i].status);
    fclose(f);
}

int isValidPassword(const char *pw) {
    for (int i = 0; pw[i]; i++) {
        if (!isalnum(pw[i])) return 0;
    }
    return 1;
}

void splitPassword(const char *pw, char *letters, char *digits) {
    int l = 0, d = 0;
    for (int i = 0; pw[i]; i++) {
        if (isalpha(pw[i])) letters[l++] = pw[i];
        else if (isdigit(pw[i])) digits[d++] = pw[i];
    }
    letters[l] = '\0';
    digits[d] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <PortNumber>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    char buff[BUFF_SIZE];
    Account accounts[100];
    int n = loadAccounts(accounts, "account.txt");

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind");
        exit(1);
    }

    printf("UDP Server running on port %d...\n", port);

    char username[50];
    int failCount = 0;
    int loggedIn = 0;
    int currentIndex = -1;

    while (1) {
        len = sizeof(cliaddr);
        memset(buff, 0, sizeof(buff));
        recvfrom(sockfd, buff, BUFF_SIZE, 0, (struct sockaddr *)&cliaddr, &len);
        buff[strcspn(buff, "\n")] = 0;

        printf("From %s:%d - %s\n", inet_ntoa(cliaddr.sin_addr),
               ntohs(cliaddr.sin_port), buff);

        if (!loggedIn) {
            int found = 0;
            for (int i = 0; i < n; i++) {
                if (strcmp(accounts[i].username, buff) == 0) {
                    found = 1;
                    currentIndex = i;
                    sendto(sockfd, "Insert password", 15, 0, (struct sockaddr *)&cliaddr, len);
                    break;
                }
            }
            if (found) {
                memset(buff, 0, sizeof(buff));
                recvfrom(sockfd, buff, BUFF_SIZE, 0, (struct sockaddr *)&cliaddr, &len);
                buff[strcspn(buff, "\n")] = 0;

                if (strcmp(accounts[currentIndex].password, buff) == 0) {
                    if (accounts[currentIndex].status == 1) {
                        sendto(sockfd, "OK", 2, 0, (struct sockaddr *)&cliaddr, len);
                        loggedIn = 1;
                        failCount = 0;
                    } else {
                        sendto(sockfd, "Account is blocked", 18, 0, (struct sockaddr *)&cliaddr, len);
                    }
                } else {
                    failCount++;
                    if (failCount >= 3) {
                        accounts[currentIndex].status = 0;
                        saveAccounts(accounts, n, "account.txt");
                        sendto(sockfd, "Account is blocked", 18, 0, (struct sockaddr *)&cliaddr, len);
                    } else {
                        sendto(sockfd, "Not OK", 6, 0, (struct sockaddr *)&cliaddr, len);
                    }
                }
            } else {
                sendto(sockfd, "Account not found", 17, 0, (struct sockaddr *)&cliaddr, len);
            }
        } else {
            if (strcmp(buff, "bye") == 0) {
                char msg[100];
                sprintf(msg, "Goodbye %s", accounts[currentIndex].username);
                sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&cliaddr, len);
                loggedIn = 0;
            } else if (strcmp(buff, "homepage") == 0) {
                sendto(sockfd, accounts[currentIndex].homepage,
                       strlen(accounts[currentIndex].homepage), 0,
                       (struct sockaddr *)&cliaddr, len);
            } else {
                if (!isValidPassword(buff)) {
                    sendto(sockfd, "Error", 5, 0, (struct sockaddr *)&cliaddr, len);
                } else {
                    strcpy(accounts[currentIndex].password, buff);
                    saveAccounts(accounts, n, "account.txt");

                    char letters[50], digits[50], response[120];
                    splitPassword(buff, letters, digits);
                    sprintf(response, "%s\n%s", letters, digits);
                    sendto(sockfd, response, strlen(response), 0,
                           (struct sockaddr *)&cliaddr, len);
                }
            }
        }
    }

    close(sockfd);
    return 0;
}
