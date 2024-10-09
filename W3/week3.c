#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#define account "nguoidung.txt"
#define history "history.txt"

typedef struct User{
    char username[50];
    char password[50];
    int status;
    char ip_or_domain[100];
    struct User* next;
} User;

User* head = NULL;
int isLog = 0;

void readUserFromAccountFile(){
    FILE *file = fopen(account, "r");
    if(file == NULL){
        printf("File nguoidung.txt doesn't exist!!!\n");
        exit(1);
    }

    printf("Loading file...\n");
    char line[200];
    while (fgets(line, sizeof(line), file)) { // đọc từng dòng từ file
        User* newUser = (User*)malloc(sizeof(User));
        if (newUser == NULL) {
            printf("Memory allocation failed!\n");
            exit(1);
        }

        // Đọc thông tin từ dòng và gán vào struct User
        sscanf(line, "%s %s %d %s",
            newUser->username,
            newUser->password,
            &newUser->status,
            newUser->ip_or_domain);

        // Thêm user mới vào danh sách liên kết
        newUser->next = head;
        head = newUser;
    }
    fclose(file);
}

void saveUsertoFile(){
    FILE *file = fopen(account, "w");
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


// Hàm kiểm tra địa chỉ ip hợp lệ
int is_valid_ip(const char *ip_in) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ip_in, &(sa.sin_addr));
    return result != 0;
}


// Tra cứu tên miền từ địa chỉ IP
void lookup_ip_to_domain(char *ip) {
    struct hostent *host;
    struct in_addr **addr_list;
    
    unsigned long ip_in_addr = inet_addr(ip);
    host = gethostbyaddr((const void *)&ip_in_addr, sizeof(ip_in_addr), AF_INET);
    
    if (host == NULL) {
        printf("No information Domain found\n");
        return;
    }
    
    printf("Main name: %s\n", host->h_name);
    
    addr_list = (struct in_addr **) host->h_addr_list;
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

// Tra cứu địa chỉ IP từ tên miền
void lookup_domain_to_ip(char *domain) {
    struct hostent *host;
    struct in_addr **addr_list;
    
    host = gethostbyname(domain);
    
    addr_list = (struct in_addr **) host->h_addr_list;
    printf("Main IP: %s\n", inet_ntoa(*addr_list[0]));
    
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


void saveLoginHistory(char username[]) {
    FILE *file = fopen(history, "a");
    if (file == NULL) {
        printf("History file doesn't exist\n\n");
        return;
    }

    time_t now = time(NULL);
    struct tm *local = localtime(&now);

    fprintf(file, "%s | %02d-%02d-%d | %02d:%02d:%02d\n",
            username,
            local->tm_mday, local->tm_mon + 1, local->tm_year + 1900, // Ngày, tháng, năm
            local->tm_hour, local->tm_min, local->tm_sec);           // Giờ, phút, giây

    fclose(file);  // Đóng file sau khi ghi
}
    void signupUser(){
        User *newUser = (User*)malloc(sizeof(User));
        struct hostent *host;
        struct in_addr **addr_list;
        printf("Username: "); scanf("%s", newUser->username);
        printf("Password: "); scanf("%s", newUser->password);
        do{
            printf("Ip or Domain: "); scanf("%s", newUser->ip_or_domain);
            if (is_valid_ip(newUser->ip_or_domain)) { // Check if it's a valid IP
                printf("This is a valid IP\n");
            } else {
                host = gethostbyname(newUser->ip_or_domain); // Check if it's a valid domain
                if (host == NULL) {
                    printf("No information Domain found\n");
                } else {
                printf("This is a valid Domain\n");
                }
            }
        } while(is_valid_ip(newUser->ip_or_domain) == 0 && host == NULL);
        newUser->status = 1;
        User *currentUser = head;
        while (currentUser != NULL){
            if(strcmp(newUser->username, currentUser->username) == 0){
                printf("Username already exists\n\n");
                free(newUser);
                return;
            }
            currentUser = currentUser->next;
        }
        newUser->next = head;
        head = newUser;
        saveUsertoFile();
        printf("Register Success!!!\n\n");
    }

User *userLoggin = NULL;
void signinUser(){
    if(isLog == 1){
        printf("Haven't exited yet!\n");
        return;
    }
    char username[50];
    printf("Enter username: "); scanf("%s", username);
    User *currentUser = head;
    int checkUsername = 0;
    while (currentUser != NULL)
    {
        if(strcmp(username, currentUser->username) == 0){
            checkUsername = 1;
            if(currentUser->status != 0){
                int count = 0;
                while(count < 3){
                    char password[50];
                    printf("Enter password: "); scanf("%s", password);
                    if(strcmp(password, currentUser->password) == 0){
                        isLog = 1;
                        saveLoginHistory(currentUser->username);
                        printf("Welcome\n\n");
                        userLoggin = currentUser;
                        return;
                    }
                    else{
                        count++;
                        printf("You have %d time to login\n", 3 - count);
                    }
                }
                if(count == 3){
                    currentUser->status = 0;
                    saveUsertoFile();
                    printf("Your account is blocked.\n\n");
                    return;
                }
                return;
            }
            else{
                printf("Your account is blocked.\n\n");
                return;
            }
        }
        currentUser = currentUser->next;
    }
    if(checkUsername == 0){
        printf("Username doesn't exist!\n\n");
    }
}

void chagePassword(){
    if (isLog == 0){
        printf("You have to sign in!\n\n");
        return;
    }
    char username[51];
    char newPass[51];
    char oldPass[51];
    printf("Old password?: ");
    scanf("%s", oldPass);
    User *currentUser = head;
    while (currentUser != NULL){
        if(strcmp(currentUser->username, userLoggin->username) == 0){
            if(strcmp(oldPass, currentUser->password) == 0){
                printf("Please type the new password: ");
                scanf("%s", newPass);
                strcpy(currentUser->password, newPass);
                saveUsertoFile();
                printf("Change password Success!!!\n\n");
                return;
            }
            else{
                printf("Wrong password, please try agian!\n\n");
                return;
            }
        }
        currentUser = currentUser->next;
    }
}

void updateAccInf(){
    struct hostent *host;
    struct in_addr **addr_list;
    if(isLog == 0){
        printf("You need to sign in!\n\n");
        return;
    }
    User *currentUser = head;
    char newIp_or_Domain[100];
    while (currentUser != NULL){
        if(strcmp(currentUser->username, userLoggin->username) == 0){
            do{
            printf("Ip or Domain: "); scanf("%s", userLoggin->ip_or_domain);
            if (is_valid_ip(userLoggin->ip_or_domain)) { // Check if it's a valid IP
            printf("This is a valid IP\n");
            } else {
                host = gethostbyname(userLoggin->ip_or_domain); // Check if it's a valid domain
                if (host == NULL) {
                    printf("No information Domain found\n");
                } else {
                printf("This is a valid Domain\n");
                }
            }
            } while(is_valid_ip(userLoggin->ip_or_domain) == 0 && host == NULL);
            saveUsertoFile();
            printf("Updated!!!\n\n");
            return;
        }
        currentUser = currentUser->next;
    }
}

char OTP[] = "19092004";
void resetPassword(){
    if(isLog == 1){
        printf("Please log out!!!\n\n");
        return;
    }
    char username[50];
    printf("Enter Username: ");
    scanf("%s", username);
    int checkUsername = 0;
    User *currentUser = head;
    while (currentUser != NULL){
        if(strcmp(currentUser->username, username) == 0){
            char otpSended[9];
            checkUsername = 1;
            do{
                printf("Please enter the OTP: "); scanf("%s", otpSended);
                if(strcmp(otpSended, OTP) == 0){
                    printf("Please type the new password: ");
                    scanf("%s", currentUser->password);
                    saveUsertoFile();
                    printf("Reset password success!!!\n\n");
                    return;
                }
                printf("Wrong OTP!!!\n\n");
            } while(strcmp(otpSended, OTP) != 0);
        }
        currentUser = currentUser->next;
    }
    if(checkUsername == 0){
        printf("Username doesn't exist\n\n");
        return;
    }
}

void logout(){
    if(isLog == 0){
        printf("Da dang nhap dau?? Ma doi dang xuat\n\n");
        return;
    }
    isLog = 0; // Cho hệ thống biết là user đã logout
    userLoggin = NULL; // Không lưu lại con trỏ loggin nữa
    printf("Logout success!!!\n\n");
}

void viewLoginHistory(char username[]) {
    FILE *file = fopen(history, "r"); // Mở file ở chế độ đọc
    if (file == NULL) {
        printf("History file doesn't exist or cannot open!!!\n");
        return;
    }

    printf("Login history for user: %s\n", username);
    printf("----------------------------------------------------\n");

    char line[200]; // Biến lưu từng dòng từ file
    while (fgets(line, sizeof(line), file)) {
        char fileUsername[50];
        char loginDate[20];
        char loginTime[20];

        // Tách thông tin username, ngày đăng nhập, và giờ đăng nhập từ file
        sscanf(line, "%s | %s | %s", fileUsername, loginDate, loginTime);

        // Xóa ký tự xuống dòng nếu có
        fileUsername[strcspn(fileUsername, "\n")] = 0;

        // So sánh username trong file với username người dùng nhập
        if (strcmp(fileUsername, username) == 0) {
            // In ra lịch sử đăng nhập theo đúng định dạng yêu cầu
            printf("%s | %s | %s\n", fileUsername, loginDate, loginTime);
        }
    }

    fclose(file);  // Đóng file sau khi đọc xong
    printf("----------------------------------------------------\n");
}

void showHomePage_IP(){
    if(isLog == 0){
        printf("You must to sign in to use this function\n");
        return;
    }
    if(is_valid_ip(userLoggin->ip_or_domain)){
        printf("IP: %s\n\n", userLoggin->ip_or_domain);
    }
    else lookup_domain_to_ip(userLoggin->ip_or_domain);
}

void showHomePage_DomainName(){
    if(isLog == 0){
        printf("You must to sign in to use this function\n");
        return;
    }
    if(!is_valid_ip(userLoggin->ip_or_domain)){
        printf("Domain name: %s\n\n", userLoggin->ip_or_domain);
    }
    else lookup_ip_to_domain(userLoggin->ip_or_domain);
}


int main(){
    readUserFromAccountFile();
    int choice;
    do{
        printf("USER MANAGEMENT PROGRAM\n");
        printf("-----------------------------------\n");
        printf("1. Register\n");
        printf("2. Sign in\n");
        printf("3. Change password\n");
        printf("4. Update account info\n");
        printf("5. Reset password\n");
        printf("6. View login history\n");
        printf("7. Homepage with domain name\n");
        printf("8. Homepage with IP address\n");
        printf("9. Log out\n");
        printf("Your choice (1-9, other to quit):\n");
        if (scanf("%d", &choice) != 1) { // Nếu nhập không phải số nguyên
            // Xử lý buffer khi nhập ký tự không hợp lệ
            while (getchar() != '\n');  // Xóa bộ đệm, chờ tới khi gặp '\n'
            printf("Exiting...\n");
            break;  // thoát
        }
        switch(choice)
        {
        case 1:
            signupUser();
            break;
        case 2:
            signinUser();
            break;
        case 3:
            chagePassword();
            break;
        case 4:
            updateAccInf();
            break;
        case 5:
            resetPassword();
            break;
        case 6:
            if (isLog == 1) {
                viewLoginHistory(userLoggin->username);
            } else {
                printf("Please sign in to view login history!\n\n");
            }
            break;
        case 7:
            showHomePage_DomainName();
            break;
        case 8:
            showHomePage_IP();
            break;
        case 9:
            logout();
            break;    
        default:
            printf("Exiting...\n");
        }
    } while(choice >= 1 && choice < 10);
}

// int checkNumberPhone(char phone[]){
//     if (strlen(phone) != 10) {
//         return 0;
//     }
//     for(int i=0; i < strlen(phone); i++){
//         if(!(phone[i] >= 48 && phone[i] <= 57)){ // SDT phải là số từ 0-9
//             return 0;
//         }
//     }
//     return 1;
// }

// int checkEmail(char email[]) {
//     int atCount = 0;   // Đếm số lượng ký tự '@'
//     int dotCount = 0;  // Đếm số lượng dấu '.'
//     int len = strlen(email);

//     // Kiểm tra email không bắt đầu hoặc kết thúc bằng dấu '.' hoặc '@'
//     if (email[0] == '.' || email[0] == '@' || email[len - 1] == '.' || email[len - 1] == '@') {
//         return 0;
//     }

//     for (int i = 0; i < len; i++) {
//         char c = email[i];

//         // Kiểm tra ký tự hợp lệ (chữ cái, số, dấu chấm, dấu gạch dưới, dấu gạch ngang, hoặc @)
//         if (!(isalnum(c) || c == '.' || c == '_' || c == '-' || c == '@')) {
//             return 0;  // Ký tự không hợp lệ
//         }

//         // Đếm số lượng '@'
//         if (c == '@') {
//             atCount++;
//             if (atCount > 1) {
//                 return 0;
//             }
//         }

//         // Đếm số lượng dấu '.'
//         if (c == '.') {
//             dotCount++;
//             // Kiểm tra không có dấu '.' liên tiếp
//             if (i > 0 && email[i - 1] == '.') {
//                 return 0;  // Dấu '.' không được liên tiếp
//             }
//         }
//     }

//     // Kiểm tra có ít nhất một '@' và một '.' sau '@' và không chứa các kí tự đặc biệt khác
//     char *atPtr = strchr(email, '@');
//     if (atPtr == NULL || strchr(atPtr, '.') == NULL || strchr(atPtr, '+') != NULL || strchr(atPtr, '-') != NULL || strchr(atPtr, '(') != NULL || strchr(atPtr, ')') != NULL || strchr(atPtr, '*') != NULL || strchr(atPtr, '/') != NULL) {
//         return 0;
//     }

//     // Kiểm tra nếu có ít nhất một ký tự hợp lệ giữa '@' và '.'
//     char *dotPtr = strchr(atPtr, '.');
//     if (dotPtr - atPtr < 2) {
//         return 0;  // Không hợp lệ nếu không có ký tự nào giữa '@' và '.'
//     }

//     return 1;  // Email hợp lệ
// }
