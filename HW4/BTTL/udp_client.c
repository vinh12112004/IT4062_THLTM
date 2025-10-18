#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define SERV_ADDR "127.0.0.1"
#define SERV_PORT 8888
#define BUFF_SIZE 1024

int main() {
    WSADATA wsa;
    SOCKET sockfd;
    int sendBytes, rcvBytes;
    int len;
    char buff[BUFF_SIZE + 1];
    struct sockaddr_in servaddr;

    //Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code: %d\n", WSAGetLastError());
        return 0;
    }

    // Step 1: Construct socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        printf("Error: Could not create socket\n");
        WSACleanup();
        return 0;
    }

    // Step 2: Define the address of the server
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERV_ADDR);
    servaddr.sin_port = htons(SERV_PORT);

    // Step 3: Communicate with server
    printf("Send to server: ");
    fgets(buff, BUFF_SIZE, stdin);
    
    buff[strcspn(buff, "\n")] = '\0';

    len = sizeof(servaddr);
    sendBytes = sendto(sockfd, buff, strlen(buff), 0, 
                      (struct sockaddr *)&servaddr, len);
    
    if (sendBytes == SOCKET_ERROR) {
        printf("Error: sendto failed\n");
        closesocket(sockfd);
        WSACleanup();
        return 0;
    }

    rcvBytes = recvfrom(sockfd, buff, BUFF_SIZE, 0, 
                       (struct sockaddr *)&servaddr, &len);
    
    if (rcvBytes == SOCKET_ERROR) {
        printf("Error: recvfrom failed\n");
        closesocket(sockfd);
        WSACleanup();
        return 0;
    }

    buff[rcvBytes] = '\0';
    printf("Reply from server: %s\n", buff);

    // Cleanup
    closesocket(sockfd);
    WSACleanup();
    
    return 0;
}