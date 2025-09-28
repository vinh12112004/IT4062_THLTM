#include "stdio.h"
#include "stdbool.h"
typedef struct Account {
    char username[50];
    char password[50];
    char email[100];
    char phone[20];
    char status[10]; // "active" hoặc "blocked"
    char role[10];   // "admin" hoặc "user" (nâng cao)
    struct Account *next;
} Account;

bool create(Account **head, const char *username, const char *password, const char *email, const char *phone, const char *status, const char *role, bool silent);
void update(Account *head, const char *username, const char *newEmail, const char *newPhone);
void read(Account *head);
void deleteAcc(Account **head, const char *username);
void saveAccountsToFile(Account *head);
void loadAccountsFromFile(Account **head);
Account* findAccount(Account *head, const char *username);