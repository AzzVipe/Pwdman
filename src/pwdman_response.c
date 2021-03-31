#include <stdio.h>
#include <unistd.h>
#include <list.h>
#include <iter.h>

#include <pwdman_response.h>
#include <resdef.h>
#include <rescodes.h>

static int pwdman_response_calc_err_index(int status);
static int pwdman_response_calc_res_index(int status);

int pwdman_response_handle_cud(struct collection *col)
{
	int index;
	struct request *req = col->request;
	struct response *res = col->response;

	res->msg = malloc(sizeof(char) * 1024);

	if((index = pwdman_response_calc_res_index(col->response->status)) != -1) {
		strcpy(res->msg, rescodes[index].desc);
		puts(res->msg);
		write(res->client, res->msg, strlen(res->msg));
	}

	return 0;

}

int pwdman_response_handle_error(struct collection *col)
{
	int index;
	struct response *res = col->response;

	res->msg = malloc(sizeof(char) * 1024);

	if((index = pwdman_response_calc_err_index(col->response->status)) != -1) {
		strcpy(res->msg, errcodes[index].desc);
		puts(res->msg);
		write(res->client, res->msg, strlen(res->msg));
	}
}

int pwdman_response_handle_print(struct collection *col)
{
	struct request *req = col->request;
	struct response *res = col->response;

	res->msg = malloc(sizeof(char) * 1 << 12);
	if (res->msg == NULL)
		return -1;
	res->msg[0] = 0;

	if (res->status == PWDMAN_SUCCESS || res->status == PWDMAN_SUCCESSFIND) {
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

	return 0;
}

static int pwdman_response_calc_err_index(int status)
{
	for (int i = 0; errcodes[i].code != 0; ++i)
	{
		if (status == errcodes[i].code) 
			return i;
	}

	return -1;
}
static int pwdman_response_calc_res_index(int status)
{
	for (int i = 0; rescodes[i].code != 0; ++i)
	{
		if (status == rescodes[i].code) 
			return i;
	}

	return -1;
}