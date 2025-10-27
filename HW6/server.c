#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MSG_TYPE_LOGIN 1
#define MSG_TYPE_TEXT  2

struct message {
    int msg_type;
    char login_name[30];
    char text[256];
};

int main() {
    int sockfd, newsock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    struct message msg;
    char filename[40];
    FILE *f;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(sockfd, 5);
    printf("Server listening...\n");

    addr_size = sizeof(client_addr);
    newsock = accept(sockfd, (struct sockaddr*)&client_addr, &addr_size);

    while (1) {
        recv(newsock, &msg, sizeof(msg), 0);
        if (msg.msg_type == MSG_TYPE_LOGIN) {
            printf("User %s logged in.\n", msg.login_name);
            sprintf(filename, "%s.txt", msg.login_name);
            f = fopen(filename, "a");
        } else if (msg.msg_type == MSG_TYPE_TEXT) {
            printf("[%s]: %s\n", msg.login_name, msg.text);
            if (f) fprintf(f, "%s\n", msg.text);
        }
    }
    close(newsock);
    close(sockfd);
}
