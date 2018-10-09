#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "error.h"
#include "authentication.h"

static UserInfo *head;

static UserInfo *user_info_alloc(void)
{
    UserInfo *node = (UserInfo *)malloc(sizeof(UserInfo));
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

    while (fgetc(file) != '\n');

    char ch = fgetc(file);
    while (!feof(file)) {
        int count = 0;

        UserInfo *node = user_info_alloc();

        while (ch != ' ' && ch != '\t') {
            node->name[count++] = ch;
            ch = fgetc(file);
        }
        node->name[count] = '\0';

        do {
            ch = fgetc(file);
        } while (ch == ' ' || ch == '\t');

        count = 0;
        while (ch != '\n' && ch != EOF) {
            node->password[count++] = ch;
            ch = fgetc(file);
        }
        node->password[count] = '\0';

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
    for (UserInfo *node = head; node != NULL; node = head) {
        head = head->next;
        free(node);
    }
}

int authentication_verify(char *name, char *password)
{
    for (UserInfo *node = head; node != NULL; node = node->next) {
        if (strcmp(name, node->name) == 0 && strcmp(password, node->password) == 0)
            return 1;
    }

    return 0;
}