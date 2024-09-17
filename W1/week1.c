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

void signupUser(){
    User *newUser = (User*)malloc(sizeof(User));
    printf("Username: "); scanf("%s", newUser->username);
    printf("Password: "); scanf("%s", newUser->password);
    printf("Email: "); scanf("%s", newUser->email);
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
            printf("Username already exists");
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
        scanf("%d", &choice);
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
            
            break;
        case 5:
            
            break;
        case 6:
            
            break;
        case 7:
            
            break;
        default:
            printf("Exiting........");
        }
    } while(choice >= 1 && choice < 8);
}