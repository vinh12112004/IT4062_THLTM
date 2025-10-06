#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ctype.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

#define MAX_INPUT 1024
#define LOG_FILE "resolver_log.txt"

// Hàm thay thế inet_pton cho IPv4 (tương thích MinGW cũ)
int my_inet_pton_v4(const char *src, void *dst) {
    struct sockaddr_in sa;
    int len = sizeof(sa);
    
    if (WSAStringToAddressA((char*)src, AF_INET, NULL, (struct sockaddr*)&sa, &len) == 0) {
        memcpy(dst, &sa.sin_addr, sizeof(struct in_addr));
        return 1;
    }
    return 0;
}

// Hàm thay thế inet_pton cho IPv6
int my_inet_pton_v6(const char *src, void *dst) {
    struct sockaddr_in6 sa;
    int len = sizeof(sa);
    
    if (WSAStringToAddressA((char*)src, AF_INET6, NULL, (struct sockaddr*)&sa, &len) == 0) {
        memcpy(dst, &sa.sin6_addr, sizeof(struct in6_addr));
        return 1;
    }
    return 0;
}

// Hàm thay thế inet_ntop cho IPv4
const char* my_inet_ntop_v4(const void *src, char *dst, size_t size) {
    struct sockaddr_in sa;
    DWORD len = size;
    
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    memcpy(&sa.sin_addr, src, sizeof(struct in_addr));
    
    if (WSAAddressToStringA((struct sockaddr*)&sa, sizeof(sa), NULL, dst, &len) == 0) {
        return dst;
    }
    return NULL;
}

// Hàm thay thế inet_ntop cho IPv6
const char* my_inet_ntop_v6(const void *src, char *dst, size_t size) {
    struct sockaddr_in6 sa;
    DWORD len = size;
    
    memset(&sa, 0, sizeof(sa));
    sa.sin6_family = AF_INET6;
    memcpy(&sa.sin6_addr, src, sizeof(struct in6_addr));
    
    if (WSAAddressToStringA((struct sockaddr*)&sa, sizeof(sa), NULL, dst, &len) == 0) {
        return dst;
    }
    return NULL;
}

// Kiểm tra địa chỉ IPv4 hợp lệ
int is_valid_ipv4(const char *ip) {
    struct in_addr addr;
    return my_inet_pton_v4(ip, &addr) == 1;
}

// Kiểm tra địa chỉ IPv6 hợp lệ
int is_valid_ipv6(const char *ip) {
    struct in6_addr addr;
    return my_inet_pton_v6(ip, &addr) == 1;
}

// Kiểm tra địa chỉ IP đặc biệt (loopback, private)
int is_special_ipv4(const char *ip) {
    struct in_addr addr;
    if (my_inet_pton_v4(ip, &addr) != 1) return 0;
    
    unsigned char *bytes = (unsigned char*)&addr.s_addr;
    
    // Loopback: 127.0.0.0/8
    if (bytes[0] == 127) return 1;
    
    // Private: 10.0.0.0/8
    if (bytes[0] == 10) return 1;
    
    // Private: 172.16.0.0/12
    if (bytes[0] == 172 && (bytes[1] >= 16 && bytes[1] <= 31)) return 1;
    
    // Private: 192.168.0.0/16
    if (bytes[0] == 192 && bytes[1] == 168) return 1;
    
    // Link-local: 169.254.0.0/16
    if (bytes[0] == 169 && bytes[1] == 254) return 1;
    
    return 0;
}

int is_special_ipv6(const char *ip) {
    struct in6_addr addr;
    if (my_inet_pton_v6(ip, &addr) != 1) return 0;
    
    // Loopback: ::1
    int is_loopback = 1;
    for (int i = 0; i < 15; i++) {
        if (addr.s6_addr[i] != 0) {
            is_loopback = 0;
            break;
        }
    }
    if (is_loopback && addr.s6_addr[15] == 1) return 1;
    
    // Link-local: fe80::/10
    if (addr.s6_addr[0] == 0xfe && (addr.s6_addr[1] & 0xc0) == 0x80) return 1;
    
    // Unique local: fc00::/7
    if ((addr.s6_addr[0] & 0xfe) == 0xfc) return 1;
    
    return 0;
}

// Ghi log
void write_log(const char *query, const char *result) {
    FILE *f = fopen(LOG_FILE, "a");
    if (f) {
        time_t now = time(NULL);
        char *timestamp = ctime(&now);
        timestamp[strlen(timestamp) - 1] = '\0';
        fprintf(f, "[%s] Query: %s\n%s\n\n", timestamp, query, result);
        fclose(f);
    }
}

// Xử lý tra cứu địa chỉ IP
void resolve_ip(const char *ip, int is_ipv6, char *log_buffer) {
    clock_t start = clock();
    
    if (!is_ipv6 && is_special_ipv4(ip)) {
        printf("Warning: Special IP address - may not have DNS record\n");
        if (log_buffer) strcat(log_buffer, "Warning: Special IP address\n");
    } else if (is_ipv6 && is_special_ipv6(ip)) {
        printf("Warning: Special IPv6 address - may not have DNS record\n");
        if (log_buffer) strcat(log_buffer, "Warning: Special IPv6 address\n");
    }
    
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = is_ipv6 ? AF_INET6 : AF_INET;
    hints.ai_flags = AI_CANONNAME;
    
    int result = getaddrinfo(ip, NULL, &hints, &res);
    
    if (result != 0 || res == NULL) {
        printf("Not found information\n");
        if (log_buffer) strcat(log_buffer, "Not found information\n");
    } else {
        char hostname[NI_MAXHOST];
        if (getnameinfo(res->ai_addr, res->ai_addrlen, hostname, sizeof(hostname), NULL, 0, 0) == 0) {
            printf("Official name: %s\n", hostname);
            if (log_buffer) {
                strcat(log_buffer, "Official name: ");
                strcat(log_buffer, hostname);
                strcat(log_buffer, "\n");
            }
            
            if (res->ai_canonname && strcmp(res->ai_canonname, hostname) != 0) {
                printf("Canonical name (CNAME): %s\n", res->ai_canonname);
                if (log_buffer) {
                    strcat(log_buffer, "Canonical name: ");
                    strcat(log_buffer, res->ai_canonname);
                    strcat(log_buffer, "\n");
                }
            }
        } else {
            printf("Not found information\n");
            if (log_buffer) strcat(log_buffer, "Not found information\n");
        }
        freeaddrinfo(res);
    }
    
    clock_t end = clock();
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    printf("Query time: %.2f ms\n", elapsed);
    if (log_buffer) {
        char time_str[50];
        sprintf(time_str, "Query time: %.2f ms\n", elapsed);
        strcat(log_buffer, time_str);
    }
}

// Xử lý tra cứu tên miền
void resolve_domain(const char *domain, char *log_buffer) {
    clock_t start = clock();
    
    struct addrinfo hints, *res, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    
    int result = getaddrinfo(domain, NULL, &hints, &res);
    
    if (result != 0 || res == NULL) {
        printf("Not found information\n");
        if (log_buffer) strcat(log_buffer, "Not found information\n");
    } else {
        int ipv4_count = 0, ipv6_count = 0;
        char ipv4_list[10][INET_ADDRSTRLEN];
        char ipv6_list[10][INET6_ADDRSTRLEN];
        
        // Hiển thị CNAME nếu có
        if (res->ai_canonname && strcmp(res->ai_canonname, domain) != 0) {
            printf("Canonical name (CNAME): %s\n", res->ai_canonname);
            if (log_buffer) {
                strcat(log_buffer, "Canonical name: ");
                strcat(log_buffer, res->ai_canonname);
                strcat(log_buffer, "\n");
            }
        }
        
        // Thu thập tất cả địa chỉ IP
        for (p = res; p != NULL; p = p->ai_next) {
            char ipstr[INET6_ADDRSTRLEN];
            void *addr;
            
            if (p->ai_family == AF_INET) {
                struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
                addr = &(ipv4->sin_addr);
                my_inet_ntop_v4(addr, ipstr, sizeof(ipstr));
                if (ipv4_count < 10) {
                    strcpy(ipv4_list[ipv4_count++], ipstr);
                }
            } else if (p->ai_family == AF_INET6) {
                struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
                addr = &(ipv6->sin6_addr);
                my_inet_ntop_v6(addr, ipstr, sizeof(ipstr));
                if (ipv6_count < 10) {
                    strcpy(ipv6_list[ipv6_count++], ipstr);
                }
            }
        }
        
        // Hiển thị IPv4
        if (ipv4_count > 0) {
            printf("Official IPv4: %s\n", ipv4_list[0]);
            if (log_buffer) {
                strcat(log_buffer, "Official IPv4: ");
                strcat(log_buffer, ipv4_list[0]);
                strcat(log_buffer, "\n");
            }
            
            if (ipv4_count > 1) {
                printf("Alias IPv4:");
                if (log_buffer) strcat(log_buffer, "Alias IPv4:");
                for (int i = 1; i < ipv4_count; i++) {
                    printf(" %s", ipv4_list[i]);
                    if (log_buffer) {
                        strcat(log_buffer, " ");
                        strcat(log_buffer, ipv4_list[i]);
                    }
                }
                printf("\n");
                if (log_buffer) strcat(log_buffer, "\n");
            }
        }
        
        // Hiển thị IPv6
        if (ipv6_count > 0) {
            printf("Official IPv6: %s\n", ipv6_list[0]);
            if (log_buffer) {
                strcat(log_buffer, "Official IPv6: ");
                strcat(log_buffer, ipv6_list[0]);
                strcat(log_buffer, "\n");
            }
            
            if (ipv6_count > 1) {
                printf("Alias IPv6:");
                if (log_buffer) strcat(log_buffer, "Alias IPv6:");
                for (int i = 1; i < ipv6_count; i++) {
                    printf(" %s", ipv6_list[i]);
                    if (log_buffer) {
                        strcat(log_buffer, " ");
                        strcat(log_buffer, ipv6_list[i]);
                    }
                }
                printf("\n");
                if (log_buffer) strcat(log_buffer, "\n");
            }
        }
        
        freeaddrinfo(res);
    }
    
    clock_t end = clock();
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    printf("Query time: %.2f ms\n", elapsed);
    if (log_buffer) {
        char time_str[50];
        sprintf(time_str, "Query time: %.2f ms\n", elapsed);
        strcat(log_buffer, time_str);
    }
}

// Xử lý một truy vấn
void process_query(const char *input) {
    char log_buffer[4096] = "";
    
    printf("\n=== Query: %s ===\n", input);
    
    if (is_valid_ipv4(input)) {
        resolve_ip(input, 0, log_buffer);
    } else if (is_valid_ipv6(input)) {
        resolve_ip(input, 1, log_buffer);
    } else {
        resolve_domain(input, log_buffer);
    }
    
    write_log(input, log_buffer);
}

// Xử lý nhiều truy vấn trên cùng một dòng
void process_multiple_queries(char *line) {
    char *token = strtok(line, " \t,;");
    while (token != NULL) {
        if (strlen(token) > 0) {
            process_query(token);
        }
        token = strtok(NULL, " \t,;");
    }
}

// Chế độ batch: đọc từ file
void batch_mode(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("Cannot open file: %s\n", filename);
        return;
    }
    
    char line[MAX_INPUT];
    printf("=== Batch mode: reading from %s ===\n", filename);
    
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = 0;
        
        if (strlen(line) == 0 || line[0] == '#') continue;
        
        process_multiple_queries(line);
    }
    
    fclose(f);
    printf("\n=== Batch processing completed ===\n");
}

// Chế độ tương tác
void interactive_mode() {
    char input[MAX_INPUT];
    
    printf("DNS Resolver - Advanced Version\n");
    printf("Enter IP address or domain name (empty line to quit)\n");
    printf("You can enter multiple addresses/domains on one line (separated by space, comma, or semicolon)\n");
    printf("All queries are logged to: %s\n\n", LOG_FILE);
    
    while (1) {
        printf("Query> ");
        if (!fgets(input, sizeof(input), stdin)) break;
        
        input[strcspn(input, "\r\n")] = 0;
        
        if (strlen(input) == 0) {
            printf("Goodbye!\n");
            break;
        }
        
        process_multiple_queries(input);
    }
}

int main(int argc, char *argv[]) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }
    
    if (argc == 2) {
        batch_mode(argv[1]);
    } else {
        interactive_mode();
    }
    
    WSACleanup();
    return 0;
}