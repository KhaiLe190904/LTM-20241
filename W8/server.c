#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/select.h>

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
    if(file == NULL){
        printf("File doesn't exist!!!\n");
        exit(1);
    }
    User *current = head;
    while(current != NULL){
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

void notifyClientsOfPasswordChange(char* username, char* newPassword, int* client_sockets, int max_sd) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "Password for %s has been updated to %s", username, newPassword);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] > 0) {
            send(client_sockets[i], buffer, strlen(buffer), 0);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: ./server PortNumber\n");
        return 1;
    }

    int port = atoi(argv[1]);
    int listening_socket, max_sd, new_socket, client_sockets[MAX_CLIENTS] = {0};
    int activity, valread;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    fd_set readfds;

    char buffer[BUFFER_SIZE];

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

    while (1) {
        // Xóa và thiết lập tập hợp file descriptor
        FD_ZERO(&readfds);
        FD_SET(listening_socket, &readfds);
        max_sd = listening_socket;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] > 0) {
                FD_SET(client_sockets[i], &readfds);
            }
            if (client_sockets[i] > max_sd) {
                max_sd = client_sockets[i];
            }
        }

        // Chờ hoạt động từ bất kỳ socket nào
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("Select error");
            continue;
        }

        // Xử lý kết nối mới
        if (FD_ISSET(listening_socket, &readfds)) {
            new_socket = accept(listening_socket, (struct sockaddr*)&client_addr, &addr_len);
            if (new_socket < 0) {
                perror("Accept failed");
                continue;
            }
            printf("New connection established: socket %d\n", new_socket);

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    break;
                }
            }
        }

        // Kiểm tra hoạt động từ các client
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];

            if (FD_ISSET(sd, &readfds)) {
                memset(buffer, 0, BUFFER_SIZE);
                valread = read(sd, buffer, BUFFER_SIZE);

                if (valread == 0) {
                    // Client ngắt kết nối
                    printf("Client on socket %d disconnected\n", sd);
                    close(sd);
                    client_sockets[i] = 0;
                } else {
                    // Xử lý yêu cầu từ client
                    buffer[valread] = '\0';
                    printf("Received from socket %d: %s\n", sd, buffer);

                    char username[50], password[50];
                    sscanf(buffer, "%s %s", username, password);

                    User* user = authenticate(username, password);
                    if (user == NULL) {
                        send(sd, "not OK", strlen("not OK"), 0);
                    } else {
                        if (user->status == 1) {
                            send(sd, "OK", strlen("OK"), 0);

                            while (1) {
                                memset(buffer, 0, BUFFER_SIZE);
                                recv(sd, buffer, BUFFER_SIZE, 0);

                                // Kiểm tra nếu client yêu cầu signout
                                if (strcmp(buffer, "bye") == 0) {
                                    printf("User %s signed out.\n", username);
                                    send(sd, "Signed out", strlen("Signed out"), 0);
                                    break;
                                } 
                                // Kiểm tra nếu client yêu cầu hiển thị homepage
                                else if (strcmp(buffer, "homepage") == 0) {
                                    printf("User %s requested homepage.\n", username);
                                    send(sd, user->ip_or_domain, strlen(user->ip_or_domain), 0);
                                } 
                                // Yêu cầu đổi mật khẩu
                                else {
                                    printf("User %s requested password change.\n", username);

                                    // Kiểm tra tính hợp lệ của mật khẩu
                                    if (!isValidPassword(buffer)) {
                                        send(sd, "Invalid password", strlen("Invalid password"), 0);
                                    } else {
                            printf("User %s requested password change.\n", username);

                            // Kiểm tra tính hợp lệ của mật khẩu
                            if (!isValidPassword(buffer)) {
                                send(sd, "Invalid password", strlen("Invalid password"), 0);
                            } else {
                                    char letters[BUFFER_SIZE] = {0}, digits[BUFFER_SIZE] = {0};
                                    encryptPassword(buffer, letters, digits);

                                    // Gửi phản hồi cho client với thông báo
                                    snprintf(buffer, BUFFER_SIZE, "New password: Letters: %s, Digits: %s", letters, digits);
                                    send(sd, buffer, strlen(buffer), 0);

                                    // Ghép mật khẩu lại thành chuỗi đầy đủ: "letters + digits"
                                    char new_password[BUFFER_SIZE] = {0};
                                    snprintf(new_password, BUFFER_SIZE, "%s%s", letters, digits);

                                    // Cập nhật mật khẩu mới cho user
                                    strcpy(user->password, new_password);
                                    saveUsertoFile();  // Lưu thông tin vào file sau khi cập nhật
                                    printf("Password for user %s has been updated to %s.\n", username, new_password);
                                    notifyClientsOfPasswordChange(username, buffer, client_sockets, max_sd);
                                    }
                                }
                                }
                            }
                        } else {
                            send(sd, "Account is blocked", strlen("Account is blocked"), 0);
                        }
                    }
                }
            }
        }
    }

    return 0;
}
