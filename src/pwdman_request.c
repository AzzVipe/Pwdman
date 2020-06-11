#include <stdio.h>
#include <string.h>

#include <pwdman_request.h>
#include <pwdman.h>
#include <validator.h>
#include <list.h>
#include <iter.h>

#include <resdef.h>

#define FIND_BY_ID    0
#define FIND_BY_SITE  1
#define FIND_BY_EMAIL 2

static int pwdman_request_find(const char *type);
static int pwdman_request_handle_id(struct collection *col);
static int pwdman_request_handle_site(struct collection *col);
static int pwdman_request_handle_email(struct collection *col);

int pwdman_request_handle_add(struct collection *col)
{
	struct request *req = col->request;
	// Validation

	if (req == NULL) {
		return -1;
	}

	if (!validate_email(request_param_get(req, "email"))) {
		col->response->status = PWDMAN_ERRINVALIDEMAIL;
		return -1;
	}

	if (!(validate_site(request_param_get(req, "site")))) {
		col->response->status = PWDMAN_ERRINVALIDSITE;
		return -1;
	}

	if(!pwdman_add(req)) {
		col->response->status = PWDMAN_ERROR;		
		return -1;
	}

	col->response->status = PWDMAN_SUCCESSADD;

	return 0;
}


int pwdman_request_handle_del(struct collection *col)
{
	struct request *req = col->request;
	// Validation
	if (atoi((request_param_get(req, "id"))) < 0) {
		col->response->status = PWDMAN_ERRINVALIDID;
		return -1;
	}

	if(!pwdman_count_by_id(req)) {
		col->response->status = PWDMAN_ERRIDNOTFOUND;
		return -1;
	}

	if(!pwdman_delete(req)) {
		col->response->status = PWDMAN_ERROR;
		return -1;
	}

	col->response->status = PWDMAN_SUCCESSDELETE;

	return 0;
}

int pwdman_request_handle_update(struct collection *col)
{
	struct request *req = col->request;
	// Validation

	if (!validate_email(request_param_get(req, "email"))) {
		col->response->status = PWDMAN_ERRINVALIDEMAIL;
		return -1;
	}

	if (!(validate_site(request_param_get(req, "site")))) {
		col->response->status = PWDMAN_ERRINVALIDSITE;
		return -1;
	}

	if(!pwdman_update(req)) {
		col->response->status = PWDMAN_ERROR;
		return -1;
	}

	col->response->status = PWDMAN_SUCCESSUPDATE;

	return 0;
}

int pwdman_request_handle_find(struct collection *col)
{
	struct request *req = col->request;
	struct response *res = col->response;
	int find;
	char *comp = request_param_get(req, "type");

	if ((find = pwdman_request_find(comp)) == -1) {
		col->response->status = PWDMAN_ERRINVALIDPARAMS;
		return -1;
	}

	res->list = list_new(sizeof(struct pwdman), NULL);

	switch(find) 
	{
		case FIND_BY_ID:
			if (pwdman_request_handle_id(col) == -1)
				return -1;

			break;

		case FIND_BY_SITE:
			if (pwdman_request_handle_site(col) == -1)
				return-1;

			break;

		case FIND_BY_EMAIL:
			if (pwdman_request_handle_email(col) == -1)
				return -1;

			break;
	}

	col->response->status = PWDMAN_SUCCESSFIND;

	return 0;
}

int pwdman_request_handle_print(struct collection *col)
{
	if ((col->response->list = pwdman_print_all()) == NULL) {
		col->response->status = PWDMAN_ERROR;
		return -1;
	}
	
	col->response->status = PWDMAN_SUCCESS;
	return 0;
}

static int pwdman_request_find(const char *type)
{
	const char finds[][32] = { 
		"id",
		"site",
		"email"
	};

	for (int i = 0; finds[i]; ++i)
	{
		if (strcmp(type, finds[i]) == 0)
			return i;
	}

	return -1;
}

static int pwdman_request_handle_id(struct collection *col)
{
	if (!validate_number(request_param_get(col->request, "id"))) {
		col->response->status = PWDMAN_ERRINVALIDID;
		return -1;
	}

	if(!pwdman_find_by_id(col->request, col->response->list)) {
		col->response->status = PWDMAN_ERRIDNOTFOUND;
		return -1;
	}
}

static int pwdman_request_handle_site(struct collection *col)
{

	if (!(validate_site(request_param_get(col->request, "site")))) {
		col->response->status = PWDMAN_ERRINVALIDSITE;
		return -1;
	}

	if(!pwdman_find_by_site(col->request, col->response->list)) {
		col->response->status = PWDMAN_ERRSITENOTFOUND;
		return -1;
	}
}

static int pwdman_request_handle_email(struct collection *col)
{

	if (!(validate_email(request_param_get(col->request, "email")))) {
		col->response->status = PWDMAN_ERRINVALIDEMAIL;
		return -1;
	}

	if(!pwdman_find_by_email(col->request, col->response->list)) {
		col->response->status = PWDMAN_ERREMAILNOTFOUND;
		return -1;
	}
}
