#ifndef __RESPONSE_H
#define __RESPONSE_H 

#include <pwdman.h>

int pwdman_response_handle_cud(struct collection *col);
int pwdman_response_handle_error(struct collection *col);
int pwdman_response_handle_print(struct collection *col);



;

#endif