#ifndef __USER_H_
#define __USER_H_ 

#include <request.h>

struct user {
	int id;
	char *username;
	char *password;
};

#define USER_TABLE "user"

#define USER_ADD    1
#define USER_UPDATE 2
#define USER_DELETE 3

void user_init(void);

#endif