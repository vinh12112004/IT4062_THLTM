#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: ./client <IPAddress> <PortNumber>\n");
        return 1;
    }

    int sock;
    struct sockaddr_in server;
    char buffer[MAX], message[MAX];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server.sin_addr);

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("Connect failed");
        return 1;
    }

    printf("Connected to server.\n");

    printf("Username: ");
    fgets(message, MAX, stdin);
    send(sock, message, strlen(message), 0);

    memset(buffer, 0, MAX);
    recv(sock, buffer, MAX, 0);
    printf("%s\n", buffer);

    if (strstr(buffer, "Account not found")) {
        close(sock);
        return 0;
    }

    while (1) {
        memset(buffer, 0, MAX);

        printf("Enter message (empty to quit): ");
        fgets(message, MAX, stdin);
        if (message[0] == '\n') break;

        send(sock, message, strlen(message), 0);

        
        recv(sock, buffer, MAX, 0);
        printf("%s\n", buffer);

        if (strstr(message,"bye") || strstr(buffer, "Account not ready") || strstr(buffer, "Account is blocked") ) break;  

        
    }
    close(sock);
    return 0;
}
