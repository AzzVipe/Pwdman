#ifndef __COMMAND_H
#define __COMMAND_H 

#include <pwdman.h>

#define BY_URI 0  /* flag */
#define BY_CMD 1  /* flag */

struct command
{
	char *cmd;
	char *uri;
	int  (*cmd_handle)(struct request *, const char **, int); 
	int  (*req_handle)(struct collection *);
	int  (*res_handle)(struct collection *);
};

extern const struct command commands[];

int  command_get_index(const char *uri, int flags);

int  command_handle(struct request *req, const char **argv, int argc, char *buf);
int  command_handle_del(struct request *req, const char **argv, int argc);
int  command_handle_update(struct request *req, const char **argv, int argc);
int  command_handle_find(struct request *req, const char **argv, int argc);
int  command_handle_add(struct request *req, const char **argv, int argc);
int  command_handle_print(struct request *req, const char **argv, int argc);



#endif