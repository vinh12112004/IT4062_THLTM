#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ctype.h>

#pragma comment(lib, "ws2_32.lib")

int is_valid_ip(const char *ip) {
    struct in_addr addr;
    return inet_addr(ip) != INADDR_NONE; //Nếu địa chỉ không hợp lệ, inet_addr() trả về INADDR_NONE = 0xFFFFFFFF`.
}

int main(int argc, char *argv[]) {
    // winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    if (argc != 2) {
        printf("Usage: %s <domain or IP>\n", argv[0]);
        return 1;
    }

    char *param = argv[1];

    if (is_valid_ip(param)) {
        // Parameter is an IPv4 address
        struct in_addr addr;
        addr.s_addr = inet_addr(param);

        struct hostent *he = gethostbyaddr((const char*)&addr, sizeof(addr), AF_INET);
        if (he == NULL) {
            printf("Not found information\n");
        } else {
            printf("Official name: %s\n", he->h_name);
            if (he->h_aliases[0] != NULL) {
                printf("Alias name:");
                for (int i = 0; he->h_aliases[i] != NULL; i++)
                    printf(" %s", he->h_aliases[i]);
                printf("\n");
            }
        }
    } else {
        // Parameter is a domain name
        struct hostent *he = gethostbyname(param);
        if (he == NULL || he->h_addr_list[0] == NULL) {
            printf("Not found information\n");
        } else {
            printf("Official IP: %s\n", inet_ntoa(*(struct in_addr*)he->h_addr_list[0]));
            if (he->h_addr_list[1] != NULL) {
                printf("Alias IP:");
                for (int i = 1; he->h_addr_list[i] != NULL; i++) {
                    printf(" %s", inet_ntoa(*(struct in_addr*)he->h_addr_list[i]));
                }
                printf("\n");
            }
        }
    }

    WSACleanup();
    return 0;
}
