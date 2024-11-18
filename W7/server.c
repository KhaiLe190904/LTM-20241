#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>

#define BUFFER_SIZE 1024
#define ACCOUNT_FILE "account.txt"

typedef struct User {
    char username[50];
    char password[50];
    int status; // 1: active, 0: blocked
    int attempts;
    char ip_or_domain[100];
    struct User* next;
} User;

User* head = NULL;

// Mutex for thread-safe access to shared data
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void readUserFromAccountFile() {
    FILE *file = fopen(ACCOUNT_FILE, "r");
    if (file == NULL) {
        printf("File account.txt doesn't exist!\n");
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
    pthread_mutex_lock(&mutex); // Lock the mutex
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
    pthread_mutex_unlock(&mutex); // Unlock the mutex
}

User* authenticate(char* username, char* password) {
    pthread_mutex_lock(&mutex); // Lock the mutex
    User* current = head;
    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            if (strcmp(current->password, password) == 0) {
                current->attempts = 0;  
                pthread_mutex_unlock(&mutex); // Unlock the mutex
                return current;
            } else {
                current->attempts++;
                if (current->attempts >= 3) {
                    current->status = 0; // Block account
                    saveUsertoFile();
                }
                pthread_mutex_unlock(&mutex); // Unlock the mutex
                return NULL;
            }
        }
        current = current->next;
    }
    pthread_mutex_unlock(&mutex); // Unlock the mutex
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

void* handleClient(void* arg) {
    int new_sockfd = *(int*)arg;
    free(arg); // Free the allocated memory for the socket descriptor

    char buffer[BUFFER_SIZE];
    
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(new_sockfd, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            printf("Client disconnected or error occurred\n");
            break;
        }

        char username[50], password[50];
        sscanf(buffer, "%s %s", username, password);
        printf("Login attempt: %s %s\n", username, password);

        User* user = authenticate(username, password);
        if (user == NULL) {
            send(new_sockfd, "not OK", strlen("not OK"), 0);
        } else {
            if (user->status == 1) {
                send(new_sockfd, "OK", strlen("OK"), 0);

                while (1) {
                    memset(buffer, 0, BUFFER_SIZE);
                    recv(new_sockfd, buffer, BUFFER_SIZE, 0);

                    // Check if client requests signout
                    if (strcmp(buffer, "bye") == 0) {
                        printf("User %s signed out.\n", username);
                        send(new_sockfd, "Signed out", strlen("Signed out"), 0);
                        break;
                    } 
                    // Check if client requests homepage
                    else if (strcmp(buffer, "homepage") == 0) {
                        printf("User %s requested homepage.\n", username);
                        send(new_sockfd, user->ip_or_domain, strlen(user->ip_or_domain), 0);
                    } 
                    // Password change request
                    else {
                        printf("User %s requested password change.\n", username);

                        // Check validity of the new password
                        if (!isValidPassword(buffer)) {
                            send(new_sockfd, "Invalid password", strlen("Invalid password"), 0);
                        } else {
                            char letters[BUFFER_SIZE] = {0}, digits[BUFFER_SIZE] = {0};
                            encryptPassword(buffer, letters, digits);

                            // Send response to client with notification
                            snprintf(buffer, BUFFER_SIZE, "New password: Letters: %s, Digits: %s", letters, digits);
                            send(new_sockfd, buffer, strlen(buffer), 0);

                            // Reconstruct the password: "letters + digits"
                            char new_password[BUFFER_SIZE] = {0};
                            snprintf(new_password, BUFFER_SIZE, "%s%s", letters, digits);

                            // Update the user's new password
                            // pthread_mutex_lock(&mutex); // Lock the mutex đang lỗi
                            strcpy(user->password, new_password);
                            saveUsertoFile();
                            // pthread_mutex_unlock(&mutex); // Unlock the mutex đang lỗi nên comment lại
                            printf("Password for user %s has been updated to %s.\n", username, new_password);
                        }
                    }
                }
            } else {
                send(new_sockfd, "Account not ready", strlen("Account not ready"), 0);
            }
        }
    }

    close(new_sockfd);  // Close connection with client
    pthread_exit(NULL); // Exit the thread
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./server PortNumber\n");
        return 1;
    }

    int port = atoi(argv[1]);

    // Initialize TCP server
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return 0;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return 0;
    }

    if (listen(sockfd, 5) < 0) {
        perror("Listen failed");
        close(sockfd);
        return 0;
    }

    readUserFromAccountFile();
    printf("Server is running on port %d...\n", port);

    while (1) {
        int* new_sockfd = malloc(sizeof(int));
        if (new_sockfd == NULL) {
            perror("Memory allocation failed");
            continue;
        }

        *new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_len);
        if (*new_sockfd < 0) {
            perror("Accept failed");
            free(new_sockfd);
            continue;
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handleClient, new_sockfd) != 0) {
            perror("Thread creation failed");
            free(new_sockfd);
        } else {
            pthread_detach(thread_id); // Detach the thread to automatically clean up
        }
    }

    close(sockfd);
    return 0;
}
