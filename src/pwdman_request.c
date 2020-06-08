#include <stdio.h>
#include <string.h>

#include <pwdman_request.h>
#include <pwdman.h>
#include <validator.h>
#include <list.h>
#include <iter.h>

#include <response_codes.h>

#define FIND_BY_ID    0
#define FIND_BY_SITE  1
#define FIND_BY_EMAIL 2

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

static int pwdman_request_handle_id(struct request *req, List *list)
{
	if (!validate_number(request_param_get(req, "id"))) {
		fprintf(stderr, "Invlalid ID\n");
		return -1;
	}

	if(!pwdman_find_by_id(req, list))
		return -1;
}

static int pwdman_request_handle_site(struct request *req, List *list)
{

	if (!(validate_site(request_param_get(req, "site")))) {
		fprintf(stderr, "Invlalid Site\n");
		return -1;
	}

	if(!pwdman_find_by_site(req, list))
		return -1;
}

static int pwdman_request_handle_email(struct request *req, List *list)
{

	if (!(validate_email(request_param_get(req, "email")))) {
		fprintf(stderr, "Invlalid Email\n");
		return -1;
	}

	if(!pwdman_find_by_email(req, list))
		return -1;
}

int pwdman_request_handle_add(struct collection *col)
{
	struct request *req = col->request;
	// Validation

	if (req == NULL) {
		return -1;
	}

	if (!validate_email(request_param_get(req, "email"))) {
		fprintf(stderr, "Invlalid Email\n");
		return -1;
	}

	if (!(validate_site(request_param_get(req, "site")))) {
		fprintf(stderr, "Invlalid Site\n");
		return -1;
	}

	if(!pwdman_add(req)) {
		col->response->status = PWDMAN_ERROR;		
		return -1;
	}

	col->response->status = PWDMAN_SUCCESS;

	return 0;
}


int pwdman_request_handle_del(struct collection *col)
{
	struct request *req = col->request;
	// Validation
	if (atoi((request_param_get(req, "id"))) < 0) {
		fprintf(stderr, "Invlalid ID\n");
		return -1;
	}

	if(!pwdman_count_by_id(req)) {
		fprintf(stderr, "Id not found\n");
		col->response->status = PWDMAN_ERRIDNOTFOUND;
		return -1;
	}

	if(!pwdman_delete(req)) {
		col->response->status = PWDMAN_ERROR;
		return -1;
	}

	col->response->status = PWDMAN_SUCCESS;

	return 0;
}

int pwdman_request_handle_update(struct collection *col)
{
	struct request *req = col->request;
	// Validation

	if (!validate_email(request_param_get(req, "email"))) {
		fprintf(stderr, "Invlalid Email\n");
		return -1;
	}

	if (!(validate_site(request_param_get(req, "site")))) {
		fprintf(stderr, "Invlalid Site\n");
		return -1;
	}

	if(!pwdman_update(req)) {
		col->response->status = PWDMAN_ERROR;
		return -1;
	}

	col->response->status = PWDMAN_SUCCESS;

	return 0;
}

int pwdman_request_handle_find(struct collection *col)
{
	struct request *req = col->request;
	int find;
	char *comp = request_param_get(req, "type");

	if ((find = pwdman_request_find(comp)) == -1) {
		fprintf(stderr, "Invlalid Parameter\n");
		return -1;
	}

	List *list = list_new(sizeof(struct pwdman), NULL);

	switch(find) 
	{
		case FIND_BY_ID:
			pwdman_request_handle_id(req, list);
			break;

		case FIND_BY_SITE:
			pwdman_request_handle_site(req, list);
			break;

		case FIND_BY_EMAIL:
			pwdman_request_handle_email(req, list);
			break;
	}

	struct pwdman *temp;
	Iter *iter = list_getiter(list);
	while(temp = iter_next(iter)) {
		printf("ID       : %d\n", temp->id);
		printf("Site     : %s\n", temp->site);
		printf("Email    : %s\n", temp->email);
		printf("Password : %s\n\n", temp->password);
	}

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