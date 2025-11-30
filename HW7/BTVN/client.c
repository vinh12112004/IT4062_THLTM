
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: ./client <IP> <Port>\n");
        return 1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

    char buffer[BUFF_SIZE];

    while (1) {
        printf("> ");
        fgets(buffer, BUFF_SIZE, stdin);
        send(sock, buffer, strlen(buffer), 0);

        if (strncmp(buffer, "QUIT", 4) == 0) break;

        memset(buffer, 0, BUFF_SIZE);
        int bytes = recv(sock, buffer, BUFF_SIZE, 0);
        if (bytes <= 0) break;

        printf("%s", buffer);
    }

    close(sock);
    return 0;
}
