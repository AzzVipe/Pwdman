#ifndef __PWDMAN_REQUEST_H_
#define __PWDMAN_REQUEST_H_

#include <pwdman.h>

int pwdman_request_handle_add(struct collection *col);
int pwdman_request_handle_del(struct collection *col);
int pwdman_request_handle_update(struct collection *col);
int pwdman_request_handle_find(struct collection *col);
int pwdman_request_handle_print(struct collection *col);







#endif