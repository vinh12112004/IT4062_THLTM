#include "stdio.h"
#include "stdbool.h"

typedef struct History {
    char username[50];
    char day[20];
    char time[20];
    struct History *next;
} History;

bool addHistory(History **head, const char *username, const char *day, const char *time);
void readHistory(History *head, const char *username);
void loadHistoryFromFile(History **head);
void saveHistoryToFile(History *head);