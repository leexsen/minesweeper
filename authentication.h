#ifndef __AUTHENTICATION_H__
#define __AUTHENTICATION_H__

typedef struct user_info {
    char name[20];
    char password[20];
    struct user_info *next;
} UserInfo;

extern void authentication_init(char *filename);
extern void authentication_destroy(void);
extern int authentication_verify(char *name, char *password);

#endif