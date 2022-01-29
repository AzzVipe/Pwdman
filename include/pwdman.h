#ifndef __PWDMAN_H
#define __PWDMAN_H

#include <list.h>
#include <stdbool.h>
#include <request.h>

#define BUFFSIZE   1024
#define INFO_TABLE "info"

#define PWDMAN_ADD    1
#define PWDMAN_UPDATE 2
#define PWDMAN_DELETE 3
#define PWDMAN_SEARCH 4

struct pwdman {
	int id;
	char *site;
	char *email;
	char *password;
};

struct response {
	int client;
	int status; 
	List *list;
	char *msg;
};

struct collection 
{
	struct request *request;
	struct response *response;
};


void pwdman_request_handle(char *buf, int clientfd);

bool pwdman_add(struct request *req);
bool pwdman_update(struct request *req);
bool pwdman_delete(struct request *req);
bool pwdman_find_by_email(struct request *req, List *list);
bool pwdman_find_by_site(struct request *req, List *list);
bool pwdman_find_by_id(struct request *req, List *list);
List *pwdman_print_all(void);

bool pwdman_count_by_id(struct request *req);



#endif