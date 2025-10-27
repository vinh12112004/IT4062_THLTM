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
    int sockfd;
    struct sockaddr_in server_addr;
    struct message msg;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    printf("Enter login name: ");
    fgets(msg.login_name, 30, stdin);
    msg.login_name[strcspn(msg.login_name, "\n")] = '\0';
    msg.msg_type = MSG_TYPE_LOGIN;
    send(sockfd, &msg, sizeof(msg), 0);

    while (1) {
        printf("Enter message: ");
        fgets(msg.text, 256, stdin);
        msg.text[strcspn(msg.text, "\n")] = '\0';
        msg.msg_type = MSG_TYPE_TEXT;
        send(sockfd, &msg, sizeof(msg), 0);
    }

    close(sockfd);
}
