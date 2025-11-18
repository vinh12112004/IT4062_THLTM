#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024

typedef struct {
    SOCKET clientSocket;
    struct sockaddr_in clientAddr;
} ClientInfo;

DWORD WINAPI ClientHandler(void *arg) {
    ClientInfo *info = (ClientInfo *)arg;
    SOCKET clientSocket = info->clientSocket;
    struct sockaddr_in clientAddr = info->clientAddr;

    char buffer[BUFFER_SIZE];
    int bytes;

    FILE *f = fopen("user.txt", "a");

    while ((bytes = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes] = '\0';

        fprintf(f, "User %s:%d : %s\n",
                inet_ntoa(clientAddr.sin_addr),
                ntohs(clientAddr.sin_port),
                buffer);
        fflush(f);
    }

    fclose(f);
    closesocket(clientSocket);
    free(info);  // free pointer
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s PortNumber\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSock == INVALID_SOCKET) {
        printf("Socket error!\n");
        return 1;
    }

    struct sockaddr_in serverAddr, clientAddr;
    int clientSize = sizeof(clientAddr);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(listenSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Bind failed!\n");
        return 1;
    }

    if (listen(listenSock, 10) == SOCKET_ERROR) {
        printf("Listen failed!\n");
        return 1;
    }

    printf("Server running on port %d...\n", port);

    while (1) {
        SOCKET clientSock = accept(listenSock,
                                   (struct sockaddr *)&clientAddr,
                                   &clientSize);

        if (clientSock == INVALID_SOCKET) {
            printf("Accept failed!\n");
            continue;
        }

        ClientInfo *info = malloc(sizeof(ClientInfo));
        info->clientSocket = clientSock;
        info->clientAddr = clientAddr;

        CreateThread(NULL, 0, ClientHandler, info, 0, NULL);
    }

    closesocket(listenSock);
    WSACleanup();
    return 0;
}
