#include "account.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"

bool create(Account **head, const char *username, const char *password, const char *email, const char *phone, const char *status, const char *role, bool silent) {
    if (findAccount(*head, username)) {
        if (!silent) printf("Username already exists\n");
        return false;
    }
    Account *newAccount = (Account *)malloc(sizeof(Account));
    if (!newAccount) {
        printf("Memory allocation failed\n");
        return false;
    }
    strncpy(newAccount->username, username, sizeof(newAccount->username) - 1);
    strncpy(newAccount->password, password, sizeof(newAccount->password) - 1);
    strncpy(newAccount->email, email, sizeof(newAccount->email) - 1);
    strncpy(newAccount->phone, phone, sizeof(newAccount->phone) - 1);
    strncpy(newAccount->status, status, sizeof(newAccount->status) - 1);
    strncpy(newAccount->role, role, sizeof(newAccount->role) - 1);
    newAccount->next = *head;
    *head = newAccount;
    return true;
}

void update(Account *head, const char *username, const char *newEmail, const char *newPhone) {
    Account *acc = findAccount(head, username);
    if (acc) {
        strncpy(acc->email, newEmail, sizeof(acc->email) - 1);
        acc->email[sizeof(acc->email) - 1] = '\0';
        strncpy(acc->phone, newPhone, sizeof(acc->phone) - 1);
        acc->phone[sizeof(acc->phone) - 1] = '\0';
        printf("Account updated successfully.\n");
    } else {
        printf("Account not found.\n");
    }
}

void read(Account *head) {
    Account *current = head;
    while (current) {
        printf("Username: %s, Email: %s, Phone: %s, Status: %s, Role: %s\n",
               current->username, current->email, current->phone, current->status, current->role);
        current = current->next;
    }
}

void deleteAcc(Account **head, const char *username) {
    if (!head || !*head) return;

    Account *toDelete = findAccount(*head, username);
    if (!toDelete) {
        printf("Account not found.\n");
        return;
    }

    // Nếu node cần xóa là head
    if (toDelete == *head) {
        *head = (*head)->next;
    } else {
        // Tìm node trước của node cần xóa
        Account *prev = *head;
        while (prev->next != toDelete) {
            prev = prev->next;
        }
        prev->next = toDelete->next;
    }

    free(toDelete);
    printf("Account deleted successfully.\n");
}

void saveAccountsToFile(Account *head) {
    FILE *f = fopen("account.txt", "w");
    if (!f) {
        printf("Cannot open account.txt for writing.\n");
        return;
    }
    Account *current = head;
    while (current) {
        fprintf(f, "%s %s %s %s %s %s\n", current->username, current->password, current->email, current->phone, current->status, current->role);
        current = current->next;
    }
    fclose(f);
}

void loadAccountsFromFile(Account **head) {
    FILE *f = fopen("account.txt", "r");
    if (!f) return;
    char username[50], password[50], email[100], phone[20], status[10], role[10];
    while (fscanf(f, "%s %s %s %s %s %s", username, password, email, phone, status, role) == 6) {
        create(head, username, password, email, phone, status, role, true);
    }
    fclose(f);
}

Account* findAccount(Account *head, const char *username) {
    Account *current = head;
    while (current) {
        if (strcmp(current->username, username) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}