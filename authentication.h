#ifndef __AUTHENTICATION_H__
#define __AUTHENTICATION_H__

#include <stdbool.h>

typedef struct user_info {
    char name[20];
    char password[20];
    struct user_info *next;
} UserInfo;

extern void authentication_init(char *filename);
extern void authentication_destroy(void);
extern bool authentication_isvalid(char *name, char *password);

#endif