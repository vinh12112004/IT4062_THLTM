#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")  // Link với thư viện Winsock

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
    SOCKET sockfd, newsock;
    struct sockaddr_in server_addr, client_addr;
    int addr_size;
    struct message msg;
    char filename[40];
    FILE *f = NULL;

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
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed. Error: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    listen(sockfd, 5);
    printf("Server listening on port %d...\n", PORT);

    addr_size = sizeof(client_addr);
    newsock = accept(sockfd, (struct sockaddr*)&client_addr, &addr_size);
    if (newsock == INVALID_SOCKET) {
        printf("Accept failed. Error: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    while (1) {
        int recv_size = recv(newsock, (char*)&msg, sizeof(msg), 0);
        if (recv_size <= 0) break; // Kết nối đóng hoặc lỗi

        if (msg.msg_type == MSG_TYPE_LOGIN) {
            printf("User %s logged in.\n", msg.login_name);
            sprintf(filename, "%s.txt", msg.login_name);
            f = fopen(filename, "a");
        } else if (msg.msg_type == MSG_TYPE_TEXT) {
            printf("[%s]: %s\n", msg.login_name, msg.text);
            if (f) fprintf(f, "%s\n", msg.text);
        }
    }

    if (f) fclose(f);
    closesocket(newsock);
    closesocket(sockfd);
    WSACleanup();

    return 0;
}
