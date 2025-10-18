#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <IP> <Port>\n", argv[0]);
        return 1;
    }

    int sockfd;
    struct sockaddr_in servaddr;
    socklen_t len;
    char buff[BUFF_SIZE];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    len = sizeof(servaddr);

    while (1) {
        printf("Enter message (empty to quit): ");
        fgets(buff, BUFF_SIZE, stdin);
        if (buff[0] == '\n') break;

        sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&servaddr, len);
        memset(buff, 0, sizeof(buff));
        recvfrom(sockfd, buff, BUFF_SIZE, 0, NULL, NULL);

        printf("Server: %s\n", buff);
    }

    close(sockfd);
    return 0;
}
