#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "error.h"
#include "authentication.h"

static UserList *head;

/*
 * This funcation is used to create an empty user object.
 */
static UserList *user_alloc(void)
{
    UserList *node = (UserList *)malloc(sizeof(UserList));
    if (node == NULL)
        error_exit(ERROR_MEM_ALLOC);

    node->next = NULL;
    return node;
}

/*
 * This function is used to read users' info from a file.
 * Each user's info is stored as a node in a linked list.
 * @Params:
 *  filename: the file where user info is stored
 */
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

        // read the username
        while (ch != ' ' && ch != '\t') {
            node->user.name[count++] = ch;
            ch = fgetc(file);
        }
        node->user.name[count] = '\0';

        // skip white characters until finding the first non-white character
        do {
            ch = fgetc(file);
        } while (ch == ' ' || ch == '\t');

        // reade the password
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

/*
 * Deallocate the resouces of the linked list
 */
void authentication_destroy(void)
{
    for (UserList *node = head; node != NULL; node = head) {
        head = head->next;
        free(node);
    }
}

/*
 * Determine if the user is an registered user
 * @params:
 *  name: the username to be validated
 *  password: the password to be validated
 */
bool authentication_isvalid(char *name, char *password)
{
    for (UserList *node = head; node != NULL; node = node->next) {
        if (strcmp(name, node->user.name) == 0 && strcmp(password, node->user.password) == 0)
            return true;
    }

    return false;
}