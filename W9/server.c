#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <poll.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define ACCOUNT_FILE "nguoidung.txt"

typedef struct User {
    char username[50];
    char password[50];
    int status; // 1: active, 0: blocked
    int attempts;
    char ip_or_domain[100];
    struct User* next;
} User;

User* head = NULL;

void readUserFromAccountFile() {
    FILE *file = fopen(ACCOUNT_FILE, "r");
    if (file == NULL) {
        printf("File nguoidung.txt doesn't exist!\n");
        exit(1);
    }

    char line[200];
    while (fgets(line, sizeof(line), file)) {
        User* newUser = (User*)malloc(sizeof(User));
        if (newUser == NULL) {
            printf("Memory allocation failed!\n");
            exit(1);
        }

        sscanf(line, "%s %s %d %s", newUser->username, newUser->password, &newUser->status, newUser->ip_or_domain);
        newUser->attempts = 0; // Initialize login attempts to 0
        newUser->next = head;
        head = newUser;
    }
    fclose(file);
}

void saveUsertoFile() {
    FILE *file = fopen(ACCOUNT_FILE, "w");
    if (file == NULL) {
        printf("File doesn't exist!!!\n");
        exit(1);
    }
    User *current = head;
    while (current != NULL) {
        fprintf(file, "%s %s %d %s\n",
            current->username,
            current->password,
            current->status,
            current->ip_or_domain);
        current = current->next;
    }
    fclose(file);
}

User* authenticate(char* username, char* password) {
    User* current = head;
    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            if (strcmp(current->password, password) == 0) {
                current->attempts = 0;
                return current;
            } else {
                current->attempts++;
                if (current->attempts >= 3) {
                    current->status = 0; // Block account
                    saveUsertoFile();
                }
                return NULL;
            }
        }
        current = current->next;
    }
    return NULL;
}

int isValidPassword(char* password) {
    for (int i = 0; password[i] != '\0'; i++) {
        if (!isalnum(password[i])) { // Not a letter or digit
            return 0;
        }
    }
    return 1;
}

void encryptPassword(char* password, char* letters, char* digits) {
    int l_idx = 0, d_idx = 0;
    for (int i = 0; password[i] != '\0'; i++) {
        if (isalpha(password[i])) {
            letters[l_idx++] = password[i];
        } else if (isdigit(password[i])) {
            digits[d_idx++] = password[i];
        }
    }
    letters[l_idx] = '\0';
    digits[d_idx] = '\0';
}

void notifyClientsOfPasswordChange(char* username, char* newPassword, struct pollfd* ufds, int client_count) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "Password for %s has been updated to %s", username, newPassword);
    
    for (int i = 1; i <= client_count; i++) { // Bắt đầu từ 1 vì 0 là listening socket
        if (ufds[i].fd > 0) {
            send(ufds[i].fd, buffer, strlen(buffer), 0);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: ./server PortNumber\n");
        return 1;
    }

    int port = atoi(argv[1]);
    int listening_socket, new_socket, client_count = 0;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    char buffer[BUFFER_SIZE];
    struct pollfd ufds[MAX_CLIENTS + 1]; // +1 cho listening_socket

    memset(ufds, 0, sizeof(ufds));

    // Tạo socket lắng nghe
    if ((listening_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Cấu hình địa chỉ server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(listening_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(listening_socket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    readUserFromAccountFile();
    printf("Server is running on port %d...\n", port);

    // Thiết lập ufds cho listening_socket
    ufds[0].fd = listening_socket;
    ufds[0].events = POLLIN;

    while (1) {
        int activity = poll(ufds, client_count + 1, -1);

        if (activity < 0) {
            perror("Poll error");
            continue;
        }

        // Kiểm tra kết nối mới
        if (ufds[0].revents & POLLIN) {
            new_socket = accept(listening_socket, (struct sockaddr*)&client_addr, &addr_len);
            if (new_socket < 0) {
                perror("Accept failed");
                continue;
            }

            printf("New connection established: socket %d\n", new_socket);

            // Thêm socket mới vào danh sách ufds
            if (client_count < MAX_CLIENTS) {
                client_count++;
                ufds[client_count].fd = new_socket;
                ufds[client_count].events = POLLIN;
            } else {
                printf("Maximum clients reached. Connection rejected.\n");
                close(new_socket);
            }
        }

        // Kiểm tra hoạt động từ các client
        for (int i = 1; i <= client_count; i++) {
            if (ufds[i].revents & POLLIN) {
                memset(buffer, 0, BUFFER_SIZE);
                int valread = recv(ufds[i].fd, buffer, BUFFER_SIZE, 0);

                if (valread == 0) {
                    // Client ngắt kết nối
                    printf("Client on socket %d disconnected\n", ufds[i].fd);
                    close(ufds[i].fd);

                    // Loại bỏ socket khỏi danh sách ufds
                    for (int j = i; j < client_count; j++) {
                        ufds[j] = ufds[j + 1];
                    }
                    client_count--;
                    i--;
                } else {
                    // Xử lý yêu cầu từ client
                    buffer[valread] = '\0';
                    printf("Received from socket %d: %s\n", ufds[i].fd, buffer);

                    char username[50], password[50];
                    sscanf(buffer, "%s %s", username, password);

                    User* user = authenticate(username, password);
                    if (user == NULL) {
                        send(ufds[i].fd, "not OK", strlen("not OK"), 0);
                    } else {
                        if (user->status == 1) {
                            send(ufds[i].fd, "OK", strlen("OK"), 0);

                            while (1) {
                                memset(buffer, 0, BUFFER_SIZE);
                                recv(ufds[i].fd, buffer, BUFFER_SIZE, 0);

                                // Kiểm tra nếu client yêu cầu signout
                                if (strcmp(buffer, "bye") == 0) {
                                    printf("User %s signed out.\n", username);
                                    send(ufds[i].fd, "Signed out", strlen("Signed out"), 0);
                                    break;
                                } 
                                // Kiểm tra nếu client yêu cầu hiển thị homepage
                                else if (strcmp(buffer, "homepage") == 0) {
                                    printf("User %s requested homepage.\n", username);
                                    send(ufds[i].fd, user->ip_or_domain, strlen(user->ip_or_domain), 0);
                                } 
                                // Yêu cầu đổi mật khẩu
                                else {
                                    printf("User %s requested password change.\n", username);

                                    // Kiểm tra tính hợp lệ của mật khẩu
                                    if (!isValidPassword(buffer)) {
                                        send(ufds[i].fd, "Invalid password", strlen("Invalid password"), 0);
                                    } else {
                                        char letters[BUFFER_SIZE] = {0}, digits[BUFFER_SIZE] = {0};
                                        encryptPassword(buffer, letters, digits);

                                        // Gửi phản hồi cho client với thông báo
                                        snprintf(buffer, BUFFER_SIZE, "New password: Letters: %s, Digits: %s", letters, digits);
                                        send(ufds[i].fd, buffer, strlen(buffer), 0);

                                        // Ghép mật khẩu lại thành chuỗi đầy đủ: "letters + digits"
                                        char new_password[BUFFER_SIZE];
                                        snprintf(new_password, sizeof(new_password), "%s%s", letters, digits);
                                        strcpy(user->password, new_password);

                                        // Cập nhật mật khẩu vào file
                                        saveUsertoFile();

                                        // Thông báo tới tất cả các client
                                        notifyClientsOfPasswordChange(username, new_password, ufds, client_count);
                                    }
                                }
                            }
                        } else {
                            send(ufds[i].fd, "Account is blocked", strlen("Account is blocked"), 0);
                        }
                    }
                }
            }
        }
    }

    return 0;
}
