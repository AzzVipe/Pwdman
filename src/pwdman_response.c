#include <stdio.h>
#include <unistd.h>
#include <list.h>
#include <iter.h>
#include <pwdman.h>


#include <pwdman_response.h>

int pwdman_response_handle_cud(struct collection *col)
{
	struct request *req = col->request;
	struct response *res = col->response;

	res->msg = malloc(sizeof(char) * 1024);

	if (res->status == PWDMAN_ERROR) {
		sprintf(res->msg, "Operation Failed\n");
	}

	if (res->status == PWDMAN_SUCCESS) {
		sprintf(res->msg, "Operation Suceeded\n");
	}

	if (res->status == PWDMAN_ERRIDNOTFOUND) {
		sprintf(res->msg, "ID Not Found\n");
		
	}

	if (res->msg != NULL) {
		puts(res->msg);
		write(res->client, res->msg, strlen(res->msg));
	}

}

int pwdman_response_handle_print(struct collection *col)
{
	struct request *req = col->request;
	struct response *res = col->response;

	res->msg = malloc(sizeof(char) * 10240);

	if (res->status == PWDMAN_ERROR) {
		sprintf(res->msg, "Operation Failed\n");
	}

	if (res->status == PWDMAN_SUCCESS) {
		sprintf(res->msg, "Operation Suceeded\n");
		char msg[1024];
		struct pwdman *temp;
		Iter *iter = list_getiter(res->list);
		while(temp = iter_next(iter)) {
			sprintf(msg, "ID       : %d\nSite     : %s\nEmail    : %s\nPassword : %s\n\n", temp->id, temp->site, temp->email, temp->password);
			strcat(res->msg, msg);
		}
	}

	if (res->msg != NULL) {
		puts(res->msg);
		write(res->client, res->msg, strlen(res->msg));
	}
}

int pwdman_response_handle_error()
{

}