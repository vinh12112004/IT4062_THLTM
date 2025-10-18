#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define SERV_PORT 8888
#define BUFF_SIZE 1024

int main() {
    WSADATA wsa;
    SOCKET sockfd;
    int rcvBytes, sendBytes;
    int len;
    char buff[BUFF_SIZE + 1];
    struct sockaddr_in servaddr, cliaddr;

    //Winsock
    printf("Initializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }
    printf("Winsock initialized.\n");

    //Step 1: Construct socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
        printf("Error: Could not create socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("Socket created.\n");

    //Step 2: Define the address of the server
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        printf("Error: Bind failed with error code: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }
    printf("Server started on port %d\n", SERV_PORT);

    //Step 3: Communicate with server
    while (1) { 
        len = sizeof(cliaddr);
        memset(buff, 0, sizeof(buff));
        
        rcvBytes = recvfrom(sockfd, buff, BUFF_SIZE, 0,
                            (struct sockaddr *)&cliaddr, &len);
        
        if (rcvBytes == SOCKET_ERROR) {
            printf("Error: recvfrom failed with error code: %d\n", WSAGetLastError());
            continue;
        }

        buff[rcvBytes] = '\0';
        
        printf("[%s:%d]: %s\n", inet_ntoa(cliaddr.sin_addr), 
               ntohs(cliaddr.sin_port), buff);

        struct sockaddr_in local_addr;
        int addr_len = sizeof(local_addr);
        getsockname(sockfd, (struct sockaddr *)&local_addr, &addr_len);

        char response[256];
        char server_ip_str[50];
        
        if (local_addr.sin_addr.s_addr == INADDR_ANY) {
            strcpy(server_ip_str, "127.0.0.1");
        } else {
            strcpy(server_ip_str, inet_ntoa(local_addr.sin_addr));
        }
        
        sprintf(response, "Hello %s:%d from %s:%d",
                inet_ntoa(cliaddr.sin_addr),
                ntohs(cliaddr.sin_port),
                server_ip_str,
                ntohs(local_addr.sin_port));

        sendBytes = sendto(sockfd, response, strlen(response), 0,
                          (struct sockaddr *)&cliaddr, len);
        
        if (sendBytes == SOCKET_ERROR) {
            printf("Error: sendto failed with error code: %d\n", WSAGetLastError());
        }
    }

    // Cleanup 
    closesocket(sockfd);
    WSACleanup();
    return 0;
}