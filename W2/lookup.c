#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

// Hàm kiểm tra địa chỉ ip hợp lệ
int is_valid_ip(char *ip_in) {
    unsigned long addr = inet_addr(ip_in);
    if (addr == INADDR_NONE) {
        return 0;
    }
    return 1;
}

// In các tên miền thay thế từ danh sách địa chỉ
void print_alternate_names(struct in_addr **addr_list) {
    if (addr_list[1] != NULL) {
        printf("Alternate names: ");
        for (int i = 1; addr_list[i] != NULL; i++) {
            printf("%s ", inet_ntoa(*addr_list[i]));
        }
        printf("\n");
    } else {
        printf("Alternate names: \n");
    }
}

// In các địa chỉ IP thay thế từ danh sách địa chỉ
void print_alternate_ips(struct in_addr **addr_list) {
    if (addr_list[1] != NULL) {
        printf("Alternate IPs: ");
        for (int i = 1; addr_list[i] != NULL; i++) {
            printf("%s ", inet_ntoa(*addr_list[i]));
        }
        printf("\n");
    } else {
        printf("Alternate IPs: \n");
    }
}

// Tra cứu tên miền từ địa chỉ IP
void lookup_ip_to_domain(char *ip) {
    struct hostent *host;
    struct in_addr **addr_list;
    
    if (!is_valid_ip(ip)) {
        printf("Invalid option\n");
        return;
    }
    
    unsigned long ip_in_addr = inet_addr(ip);
    host = gethostbyaddr((const void *)&ip_in_addr, sizeof(ip_in_addr), AF_INET);
    
    if (host == NULL) {
        printf("No information found\n");
        return;
    }
    
    printf("Main name: %s\n", host->h_name);
    
    addr_list = (struct in_addr **) host->h_addr_list;
    print_alternate_names(addr_list);
}

// Tra cứu địa chỉ IP từ tên miền
void lookup_domain_to_ip(char *domain) {
    struct hostent *host;
    struct in_addr **addr_list;

    if (is_valid_ip(domain)) {
        printf("Invalid option\n");
        return;
    }
    
    host = gethostbyname(domain);
    if (host == NULL) {
        printf("No information found\n");
        return;
    }
    
    addr_list = (struct in_addr **) host->h_addr_list;
    printf("Main IP: %s\n", inet_ntoa(*addr_list[0]));
    
    print_alternate_ips(addr_list);
}

// Xử lý dựa trên option
void handle_lookup_option(int option, char *parameter) {
    if (option == 1) {
        lookup_ip_to_domain(parameter);
    } else if (option == 2) {
        lookup_domain_to_ip(parameter);
    } else {
        printf("Invalid option\n");
    }
}

// Kiểm tra và xử lý tham số đầu vào
void validate_and_execute(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: ./lookup option parameter\n");
        exit(1);
    }

    int option = atoi(argv[1]);
    char *parameter = argv[2];
    
    handle_lookup_option(option, parameter);
}

// Hàm main
int main(int argc, char *argv[]) {
    validate_and_execute(argc, argv);
    return 0;
}
