#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: ./client IPAddress PortNumber\n");
        return 1;
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Tạo socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return 0;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    // Kết nối tới server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        return 0;
    }

    while (1) {
        // Nhập username và password từ người dùng
        char username[50], password[50];
        printf("Enter username (or empty to quit): ");
        fgets(username, sizeof(username), stdin);
        username[strcspn(username, "\n")] = 0; // Xóa ký tự newline

        if (strlen(username) == 0) {
            printf("Exiting...\n");
            break;
        }

        printf("Enter password: ");
        fgets(password, sizeof(password), stdin);
        password[strcspn(password, "\n")] = 0; // Xóa ký tự newline

        // Gửi thông tin đăng nhập tới server
        snprintf(buffer, sizeof(buffer), "%s %s", username, password);
        send(sockfd, buffer, strlen(buffer), 0);

        // Nhận phản hồi từ server
        memset(buffer, 0, BUFFER_SIZE);
        recv(sockfd, buffer, BUFFER_SIZE, 0);
        printf("Server response: %s\n", buffer);

        if (strcmp(buffer, "OK") == 0) {
            // Đăng nhập thành công, cho phép người dùng chọn hành động tiếp theo
            while (1) {
                printf("Choose an action: \n1. Change password\n2. Homepage\n3. Signout (bye)\n");
                printf("Enter your choice: ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = 0; // Xóa ký tự newline

                if (strcmp(buffer, "1") == 0) {
                    // Yêu cầu đổi mật khẩu
                    char new_password[BUFFER_SIZE];
                    printf("Enter new password: ");
                    fgets(new_password, sizeof(new_password), stdin);
                    new_password[strcspn(new_password, "\n")] = 0;
                    
                    if (strlen(new_password) == 0) {
                    printf("Invalid password\n");
                    continue; // Quay lại để yêu cầu nhập lại
                    }

                    send(sockfd, new_password, strlen(new_password), 0);

                    // Nhận phản hồi từ server
                    memset(buffer, 0, BUFFER_SIZE);
                    recv(sockfd, buffer, BUFFER_SIZE, 0);
                    printf("Server response: %s\n", buffer);

                } else if (strcmp(buffer, "2") == 0) {
                    // Yêu cầu homepage
                    strcpy(buffer, "homepage");
                    send(sockfd, buffer, strlen(buffer), 0);

                    // Nhận phản hồi từ server
                    memset(buffer, 0, BUFFER_SIZE);
                    recv(sockfd, buffer, BUFFER_SIZE, 0);
                    printf("Server response: %s\n", buffer);

                } else if (strcmp(buffer, "3") == 0 || strcmp(buffer, "bye") == 0) {
                    // Yêu cầu đăng xuất
                    strcpy(buffer, "bye");
                    send(sockfd, buffer, strlen(buffer), 0);

                    // Nhận phản hồi từ server và thoát vòng lặp
                    memset(buffer, 0, BUFFER_SIZE);
                    recv(sockfd, buffer, BUFFER_SIZE, 0);
                    printf("Server response: %s\n", buffer);
                    break;
                } else {
                    printf("Invalid choice, please try again.\n");
                }
            }
        } else if (strcmp(buffer, "account not ready") == 0) {
            printf("Account is blocked.\n");
        } else if (strcmp(buffer, "not OK") == 0) {
            printf("Incorrect password.\n");
        }
    }

    close(sockfd);
    return 0;
}
