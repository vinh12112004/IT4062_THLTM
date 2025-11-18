#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")  // Link thư viện Winsock

#define PORT 8080
#define MSG_TYPE_LOGIN 1
#define MSG_TYPE_TEXT  2

struct message {
    int msg_type;
    char login_name[30];
    char text[256];
};

int main() {
    WSADATA wsa;
    SOCKET sockfd;
    struct sockaddr_in server_addr;
    struct message msg;

    // Khởi tạo Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Failed. Error Code : %d\n", WSAGetLastError());
        return 1;
    }

    // Tạo socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        printf("Could not create socket : %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // localhost

    // Kết nối tới server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Connect failed. Error: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    printf("Enter login name: ");
    fgets(msg.login_name, 30, stdin);
    msg.login_name[strcspn(msg.login_name, "\n")] = '\0';
    msg.msg_type = MSG_TYPE_LOGIN;

    send(sockfd, (char*)&msg, sizeof(msg), 0);

    while (1) {
        printf("Enter message: ");
        fgets(msg.text, 256, stdin);
        msg.text[strcspn(msg.text, "\n")] = '\0';
        msg.msg_type = MSG_TYPE_TEXT;

        send(sockfd, (char*)&msg, sizeof(msg), 0);
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}
