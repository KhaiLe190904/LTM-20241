#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#define account "account.txt"
#define history "history.txt"

typedef struct User{
    char username[50];
    char password[50];
    char email[50];
    char phone[11];
    int status;
    struct User* next;
} User;

User* head = NULL;
int isLog = 0;

void readUserFromAccountFile(){
    FILE *file = fopen(account, "r");
    if(file == NULL){
        printf("File account.txt doesn't exist!!!\n");
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
        sscanf(line, "%s %s %s %s %d",
            newUser->username,
            newUser->password,
            newUser->email,
            newUser->phone,
            &newUser->status);

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
        fprintf(file, "%s %s %s %s %d\n",
        current->username,
        current->password,
        current->email,
        current->phone,
        current->status);
        current = current->next;
    }
    fclose(file);
}

int checkNumberPhone(char phone[]){
    if (strlen(phone) != 10) {
        return 0;
    }
    for(int i=0; i < strlen(phone); i++){
        if(!(phone[i] >= 48 && phone[i] <= 57)){ // SDT phải là số từ 0-9
            return 0;
        }
    }
    return 1;
}

int checkEmail(char email[]) {
    int atCount = 0;   // Đếm số lượng ký tự '@'
    int dotCount = 0;  // Đếm số lượng dấu '.'
    int len = strlen(email);

    // Kiểm tra email không bắt đầu hoặc kết thúc bằng dấu '.' hoặc '@'
    if (email[0] == '.' || email[0] == '@' || email[len - 1] == '.' || email[len - 1] == '@') {
        return 0;
    }

    for (int i = 0; i < len; i++) {
        char c = email[i];

        // Kiểm tra ký tự hợp lệ (chữ cái, số, dấu chấm, dấu gạch dưới, dấu gạch ngang, hoặc @)
        if (!(isalnum(c) || c == '.' || c == '_' || c == '-' || c == '@')) {
            return 0;  // Ký tự không hợp lệ
        }

        // Đếm số lượng '@'
        if (c == '@') {
            atCount++;
            if (atCount > 1) {
                return 0;
            }
        }

        // Đếm số lượng dấu '.'
        if (c == '.') {
            dotCount++;
            // Kiểm tra không có dấu '.' liên tiếp
            if (i > 0 && email[i - 1] == '.') {
                return 0;  // Dấu '.' không được liên tiếp
            }
        }
    }

    // Kiểm tra có ít nhất một '@' và một '.' sau '@' và không chứa các kí tự đặc biệt khác
    char *atPtr = strchr(email, '@');
    if (atPtr == NULL || strchr(atPtr, '.') == NULL || strchr(atPtr, '+') != NULL || strchr(atPtr, '-') != NULL || strchr(atPtr, '(') != NULL || strchr(atPtr, ')') != NULL || strchr(atPtr, '*') != NULL || strchr(atPtr, '/') != NULL) {
        return 0;
    }

    // Kiểm tra nếu có ít nhất một ký tự hợp lệ giữa '@' và '.'
    char *dotPtr = strchr(atPtr, '.');
    if (dotPtr - atPtr < 2) {
        return 0;  // Không hợp lệ nếu không có ký tự nào giữa '@' và '.'
    }

    return 1;  // Email hợp lệ
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
    printf("Username: "); scanf("%s", newUser->username);
    printf("Password: "); scanf("%s", newUser->password);
    do{
        printf("Email: "); scanf("%s", newUser->email);
        if(checkEmail(newUser->email) == 0){
            printf("Re-Enter!!! Email khong dung format\n");
        }
    } while(checkEmail(newUser->email) == 0);

    printf("Phone Number: "); scanf("%s", newUser->phone);
    if(checkNumberPhone(newUser->phone) == 0){
            printf("Re-Enter!!! So dien thoai phai la 10 so\n");
    }
    while(checkNumberPhone(newUser->phone) == 0){
        printf("Phone Number: "); scanf("%s", newUser->phone);
        if(checkNumberPhone(newUser->phone) == 0){
            printf("Re-Enter!!! So dien thoai phai la 10 so\n");
        }
    }
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
    if(isLog == 0){
        printf("You need to sign in!\n\n");
        return;
    }
    User *currentUser = head;
    char newEmail[50];
    char newSDT[11];
    while (currentUser != NULL){
        if(strcmp(currentUser->username, userLoggin->username) == 0){
            do{
                printf("New email: "); scanf("%s", userLoggin->email);
                if(checkEmail(userLoggin->email) == 0){
                    printf("Re-Enter!!! Email khong dung format\n");
                }
            } while(checkEmail(userLoggin->email) == 0);
            do{
                printf("New phone number: "); scanf("%s", userLoggin->phone);
                if(checkNumberPhone(userLoggin->phone) == 0){
                    printf("Re-Enter!!! So dien thoai phai la 10 so\n");
                }
            } while(checkNumberPhone(userLoggin->phone) == 0);
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
    while (currentUser != NULL ){
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
        printf("7. Sign out\n");
        printf("Your choice (1-7, other to quit):\n");
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
            logout();
            break;
        default:
            printf("Exiting...\n");
        }
        // Chờ người dùng nhấn Enter để tiếp tục
        printf("Press Enter to continue...");
        while (getchar() != '\n');  // Xóa bộ đệm
        getchar();  // Chờ người dùng nhấn Enter
    } while(choice >= 1 && choice < 8);
}
