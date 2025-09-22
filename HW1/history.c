#include "stdio.h"
#include "history.h"
#include "string.h"
#include "stdlib.h"
#include "stdbool.h"

bool addHistory(History **head, const char *username, const char *day, const char *time) {
    History *newHistory = (History *)malloc(sizeof(History));
    if (!newHistory) {
        printf("Memory allocation failed\n");
        return false;
    }
    strncpy(newHistory->username, username, sizeof(newHistory->username) - 1);
    strncpy(newHistory->day, day, sizeof(newHistory->day) - 1);
    strncpy(newHistory->time, time, sizeof(newHistory->time) - 1);
    newHistory->next = *head;
    *head = newHistory;
    return true;
}

void readHistory(History *head, const char *username) {
    History *current = head;
    bool found = false;
    while (current) {
        if (strcmp(current->username, username) == 0) {
            printf("Username: %s, Day: %s, Time: %s\n",
                   current->username, current->day, current->time);
            found = true;
        }
        current = current->next;
    }
    if (!found) {
        printf("No history found for user: %s\n", username);
    }
}

void loadHistoryFromFile(History **head) {
    FILE *file = fopen("history.txt", "r");
    if (!file) {
        printf("Could not open history file for reading.\n");
        return;
    }
    char username[50], day[20], time[20];
    while (fscanf(file, "%49s | %19s | %19s", username, day, time) == 3) {
        addHistory(head, username, day, time);
    }
    fclose(file);
}

void saveHistoryToFile(History *head) {
    FILE *file = fopen("history.txt", "w");
    if (!file) {
        printf("Could not open history file for writing.\n");
        return;
    }
    History *current = head;
    while (current) {
        fprintf(file, "%s | %s | %s\n", current->username, current->day, current->time);
        current = current->next;
    }
    fclose(file);
}