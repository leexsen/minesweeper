#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "error.h"
#include "authentication.h"

static UserList *head;

static UserList *user_alloc(void)
{
    UserList *node = (UserList *)malloc(sizeof(UserList));
    if (node == NULL)
        error_exit(ERROR_MEM_ALLOC);

    node->next = NULL;
    return node;
}

void authentication_init(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
        error_exit(OPEN_FILE_FAILED);

    // skip the this line
    while (fgetc(file) != '\n');

    char ch = fgetc(file);
    while (!feof(file)) {
        int count = 0;

        UserList *node = user_alloc();

        while (ch != ' ' && ch != '\t') {
            node->user.name[count++] = ch;
            ch = fgetc(file);
        }
        node->user.name[count] = '\0';

        do {
            ch = fgetc(file);
        } while (ch == ' ' || ch == '\t');

        count = 0;
        while (ch != '\n' && ch != EOF) {
            node->user.password[count++] = ch;
            ch = fgetc(file);
        }
        node->user.password[count] = '\0';

        if (ch == '\n')
            ch = fgetc(file);

        if (head == NULL)
            head = node;
        else {
            node->next = head;
            head = node;
        }
    }

    fclose(file);
}

void authentication_destroy(void)
{
    for (UserList *node = head; node != NULL; node = head) {
        head = head->next;
        free(node);
    }
}

bool authentication_isvalid(char *name, char *password)
{
    for (UserList *node = head; node != NULL; node = node->next) {
        if (strcmp(name, node->user.name) == 0 && strcmp(password, node->user.password) == 0)
            return true;
    }

    return false;
}