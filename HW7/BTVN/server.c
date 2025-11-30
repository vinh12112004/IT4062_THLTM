#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <sys/shm.h>

#define MAX_CLIENT 100
#define BUFF_SIZE 1024

typedef struct {
    char username[50];
    char password[50];
    int status;
} Account;

Account accounts[100];
int acc_count = 0;

// char online_users[MAX_CLIENT][50];
// int online_count = 0;

typedef struct {
    char online_users[MAX_CLIENT][50];
    int online_count;
} SharedMem;

SharedMem *shm = NULL;
int shmid;


void trim(char *s) {
    s[strcspn(s, "\n")] = 0;
}

char *timestamp() {
    static char buf[64];
    time_t now = time(NULL);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return buf;
}

void write_log(const char *msg) {
    FILE *f = fopen("auth.log", "a");
    fprintf(f, "[%s] %s\n", timestamp(), msg);
    fclose(f);
}

void load_accounts() {
    FILE *f = fopen("account.txt", "r");
    acc_count = 0;

    while (fscanf(f, "%s %s %d", accounts[acc_count].username,
                  accounts[acc_count].password,
                  &accounts[acc_count].status) != EOF) {
        acc_count++;
    }
    fclose(f);
}

void save_accounts() {
    FILE *f = fopen("account.txt", "w");
    for (int i = 0; i < acc_count; i++) {
        fprintf(f, "%s %s %d\n",
                accounts[i].username,
                accounts[i].password,
                accounts[i].status);
    }
    fclose(f);
}

int find_account(const char *username) {
    for (int i = 0; i < acc_count; i++) {
        if (strcmp(accounts[i].username, username) == 0)
            return i;
    }
    return -1;
}

void add_online(const char *user) {
    strcpy(shm->online_users[shm->online_count++], user);
}

void remove_online(const char *user) {
    for (int i = 0; i < shm->online_count; i++) {
        if (strcmp(shm->online_users[i], user) == 0) {
            for (int j = i; j < shm->online_count-1; j++)
                strcpy(shm->online_users[j], shm->online_users[j+1]);
            shm->online_count--;
            return;
        }
    }
}

void handle_client(int client_sock, struct sockaddr_in client_addr) {
    char buffer[BUFF_SIZE];
    char current_user[50] = "";
    int login_fail = 0;

    while (1) {
        memset(buffer, 0, BUFF_SIZE);
        int bytes = recv(client_sock, buffer, BUFF_SIZE, 0);
        if (bytes <= 0) break;

        trim(buffer);

        char cmd[20], user[50], pass[50];
        sscanf(buffer, "%s", cmd);

        // ========== LOGIN ==========
        if (strcmp(cmd, "LOGIN") == 0) {
            sscanf(buffer, "%*s %s %s", user, pass);

            int idx = find_account(user);
            char logbuf[256];

            if (idx == -1) {
                sprintf(logbuf, "LOGIN %s from %s:%d FAIL (no such account)",
                        user, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                write_log(logbuf);
                send(client_sock, "ERR Account does not exist\n", 30, 0);
                continue;
            }

            if (accounts[idx].status == 0) {
                sprintf(logbuf, "LOGIN %s from %s:%d FAIL (locked)",
                        user, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                write_log(logbuf);
                send(client_sock, "ERR Account locked\n", 20, 0);
                continue;
            }

            if (strcmp(pass, accounts[idx].password) != 0) {
                login_fail++;
                sprintf(logbuf, "LOGIN %s from %s:%d FAIL (wrong password)",
                        user, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                write_log(logbuf);

                if (login_fail >= 3) {
                    accounts[idx].status = 0;
                    save_accounts();
                    write_log("ACCOUNT_LOCKED user");
                    send(client_sock, "ERR Account locked due to 3 failed attempts\n", 45, 0);
                } else {
                    send(client_sock, "ERR Wrong password\n", 20, 0);
                }
                continue;
            }

            // Success
            login_fail = 0;
            strcpy(current_user, user);
            add_online(user);

            sprintf(logbuf, "LOGIN %s from %s:%d SUCCESS",
                    user, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            write_log(logbuf);

            send(client_sock, "OK Login successful\n", 22, 0);
        }

        // ========== LOGOUT ==========
        else if (strcmp(cmd, "LOGOUT") == 0) {
            if (strlen(current_user) == 0) {
                send(client_sock, "ERR Not logged in\n", 20, 0);
            } else {
                char logbuf[256];
                sprintf(logbuf, "LOGOUT %s from %s:%d",
                        current_user, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                write_log(logbuf);

                remove_online(current_user);
                memset(current_user, 0, sizeof(current_user));

                send(client_sock, "OK Logged out\n", 15, 0);
            }
        }

        // ========== WHO ==========
        else if (strcmp(cmd, "WHO") == 0) {
            char resp[1024] = "LIST ";
            for (int i = 0; i < shm->online_count; i++) {
                strcat(resp, shm->online_users[i]);
                strcat(resp, " ");
            }
            strcat(resp, "\n");
            send(client_sock, resp, strlen(resp), 0);
        }

        // ========== HELP ==========
        else if (strcmp(cmd, "HELP") == 0) {
            char *msg =
                "Supported commands:\n"
                "LOGIN <user> <pass>\n"
                "LOGOUT\n"
                "WHO\n"
                "HELP\n"
                "QUIT\n";
            send(client_sock, msg, strlen(msg), 0);
        }

        // ========== QUIT ==========
        else if (strcmp(cmd, "QUIT") == 0) {
            break;
        }

        else {
            send(client_sock, "ERR Unknown command\n", 22, 0);
        }
    }

    close(client_sock);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./server <Port>\n");
        return 1;
    }
    // Tạo shared memory chứa danh sách online
    shmid = shmget(IPC_PRIVATE, sizeof(SharedMem), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    // ánh xạ vào vùng nhớ
    shm = (SharedMem*) shmat(shmid, NULL, 0);
    if (shm == (void*) -1) {
        perror("shmat");
        exit(1);
    }

    // khởi tạo giá trị ban đầu
    shm->online_count = 0;


    int port = atoi(argv[1]);
    load_accounts();

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_sock, 10);

    printf("Server running on port %d\n", port);

    while (1) {
        int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);

        if (fork() == 0) {
            close(server_sock);
            handle_client(client_sock, client_addr);
        }

        close(client_sock);
    }

    return 0;
}