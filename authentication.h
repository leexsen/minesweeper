#ifndef __AUTHENTICATION_H__
#define __AUTHENTICATION_H__

#include <stdbool.h>

typedef struct {
    char name[20];
    char password[20];
} User;

typedef struct user_list {
    User user;
    struct user_list *next;
} UserList;

extern void authentication_init(char *filename);
extern void authentication_destroy(void);
extern bool authentication_isvalid(char *name, char *password);

#endif