#include "stdio.h"
#include "account.h"
#include "string.h"
#include "history.h"
#include "time.h"

History *historyList = NULL;
Account *accountList = NULL;
Account *currentUser = NULL;
int verification_code = 121124;

void getCurrentDateTime(char *dateStr, size_t dateSize, char *timeStr, size_t timeSize) {
    time_t t;
    time(&t);

    struct tm *local = localtime(&t);

    strftime(dateStr, dateSize, "%d-%m-%Y", local);
    strftime(timeStr, timeSize, "%H:%M", local);
}

void showMenu(){
    printf("USER MANAGEMENT PROGRAM\n");
    printf(" -----------------------------------  \n");
    printf("1. Register \n");
    printf("2. Sign in  \n");
    printf("3. Change password \n");
    printf("4. Update account info \n");
    printf("5. Reset password \n");
    printf("6. View login history \n");
    printf("7. Sign out \n");
    printf("Your choice (1-7, other to quit): ");
}

void registerAccount(){
    char username[50], password[50], email[100], phone[20];
    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);
    printf("Enter email: ");
    scanf("%s", email);
    printf("Enter phone: ");
    scanf("%s", phone);
    if( create(&accountList, username, password, email, phone, "active", "user") ) 
        saveAccountsToFile(accountList);
}

void signIn(){
    char username[50], password[50];
    printf("Enter username: ");
    scanf("%s", username);
    Account *acc = findAccount(accountList, username);
    if(!acc){
        printf("Account not found.\n");
        return;
    }
    int wrongCount=0;
    while (wrongCount<3)
    {
        printf("Enter password: ");
        scanf("%s", password);

        if(strcmp(acc->password, password)==0){
            printf("Sign in successful.\n");
            currentUser = acc;
            // set current date and time
            char dateStr[20], timeStr[20];
            getCurrentDateTime(dateStr, sizeof(dateStr), timeStr, sizeof(timeStr));
            if(addHistory(&historyList, currentUser->username, dateStr, timeStr))
                saveHistoryToFile(historyList);

            wrongCount=0;
            return;
        } else {
            wrongCount++;
            printf("Incorrect password. You have %d attempt(s) left.\n", 3-wrongCount);
        }
    }

    strcpy(acc->status, "blocked");
    saveAccountsToFile(accountList);
    printf("Your account is blocked.\n");
}

void changePassword(){
    if(!currentUser){
        printf("No user is currently signed in.\n");
        return;
    }
    char oldPassword[50], newPassword[50];
    printf("Enter old password: ");
    scanf("%s", oldPassword);
    if(strcmp(currentUser->password, oldPassword) != 0){
        printf("Old password is incorrect.\n");
        return;
    }
    printf("Enter new password: ");
    scanf("%s", newPassword);
    strcpy(currentUser->password, newPassword);
    saveAccountsToFile(accountList);
    printf("Password changed successfully.\n");
}

void signOut(){
    if(!currentUser){
        printf("No user is currently signed in.\n");
        return;
    }
    currentUser = NULL;
    printf("Signed out successfully.\n");
}

void updateAccountInfo(){
    if(!currentUser){
        printf("No user is currently signed in.\n");
        return;
    }
    char newEmail[100], newPhone[20];
    printf("Enter new email: ");
    scanf("%s", newEmail);
    printf("Enter new phone: ");
    scanf("%s", newPhone);
    if(strlen(newEmail) > 0)
        strcpy(currentUser->email, newEmail);
    if(strlen(newPhone) > 0)
        strcpy(currentUser->phone, newPhone);
    saveAccountsToFile(accountList);
    printf("Account info updated successfully.\n");
}

void resetPassword(){
    char username[50];
    printf("Enter username for password reset: ");
    scanf("%s", username);
    Account *acc = findAccount(accountList, username);
    if(!acc){
        printf("Account not found.\n");
        return;
    }
    int code;
    printf("Enter verification code sent to your email/phone: ");
    scanf("%d", &code);
    if(code != verification_code){
        printf("Incorrect verification code.\n");
        return;
    }
    char newPassword[50];
    printf("Enter new password: ");
    scanf("%s", newPassword);
    strcpy(acc->password, newPassword);
    saveAccountsToFile(accountList);
    printf("Password reset successfully.\n");
}

void viewLoginHistory(){
    if(!currentUser){
        printf("No user is currently signed in.\n");
        return;
    }
    readHistory(historyList, currentUser->username);
}

void init(){
    loadAccountsFromFile(&accountList);
    loadHistoryFromFile(&historyList);
}
int main(){
    init();
    while (1)
    {
        showMenu();
        int choice;
        scanf("%d", &choice);
        switch (choice)
        {
        case 1:
            registerAccount();
            break;
        case 2:
            signIn();
            break;
        case 3:
            changePassword();
            break;
        case 4:
            updateAccountInfo();
            break;
        case 5:
            resetPassword();
            break;
        case 6:
            viewLoginHistory();
            break;
        case 7:
            signOut();
            break;
        default:
            printf("Exiting program.\n");
            return 0;
        }
    }
    
    return 0;
}