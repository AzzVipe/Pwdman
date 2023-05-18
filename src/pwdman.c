#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<iter.h>
#include<pwdman.h>
#include<database.h>
#include<command.h>
#include<pwdman_response.h>

static bool pwdman_storage_add(struct pwdman *temp);
static bool pwdman_storage_update(struct pwdman *temp);
static bool pwdman_get_list(List *list);
static void pwdman_prepare_struct(struct request *req, struct pwdman *temp, int state);
static void destroyer(void *data);

int callback(void *list, int count, char **data, char **columns);

void pwdman_request_handle(char *buf, int clientfd)
{
	int index;
	struct request req = {0};
	struct response res = {0};
	struct collection col = {&req, &res};
	char *uri;

	request_parse(buf, strlen(buf), &req);
	uri = request_uri_get(&req);
	res.client = clientfd;

	// check the uri and invoke the corresponding req handler
	if ((index = command_get_index(uri, BY_URI)) == -1) {
		fprintf(stderr, "Invaild Uri : %s\n", uri);
		return;
	}

	// call handler
	if(commands[index].req_handle(&col) == -1) {
		fprintf(stderr, "request_handle : Error\n" );
		pwdman_response_handle_error(&col);
		return;
	}
	
	// responses
	if(commands[index].res_handle(&col) == -1) {
		fprintf(stderr, "response_handle : Error\n" );
		return;
	}

}

bool pwdman_add(struct request *req)
{
	struct pwdman temp = {0};

	pwdman_prepare_struct(req, &temp, PWDMAN_ADD);

	return pwdman_storage_add(&temp);
}

bool pwdman_update(struct request *req)
{
	struct pwdman temp;

	pwdman_prepare_struct(req, &temp, PWDMAN_UPDATE);

	return pwdman_storage_update(&temp);

}

bool pwdman_delete(struct request *req)
{
	int id;
	char sql[BUFFSIZE];
	const char sql_buf[] = "DELETE FROM " INFO_TABLE " WHERE id = %d";

	id = atoi(request_param_get(req, "id"));
	sprintf(sql, sql_buf, id);

	return database_cud(sql);
}

bool pwdman_find_by_email(struct request *req, List *list)
{
	char sql[BUFFSIZE];
	const char sql_buf[] = "SELECT * FROM " INFO_TABLE " WHERE email LIKE '%%%s%%' ";

	sprintf(sql, sql_buf, request_param_get(req, "email"));

	return database_select(sql, (void *)list, callback);

}

bool pwdman_find_by_site(struct request *req, List *list)
{
	char sql[BUFFSIZE];
	const char sql_buf[] = "SELECT * FROM " INFO_TABLE " WHERE site LIKE '%%%s%%' ";

	sprintf(sql, sql_buf, request_param_get(req, "site"));

	return database_select(sql, (void *)list, callback) && list->size;
}

bool pwdman_find_by_id(struct request *req, List *list)
{
	char sql[BUFFSIZE];
	const char sql_buf[] = "SELECT * FROM " INFO_TABLE " WHERE id=%d";

	sprintf(sql, sql_buf, atoi(request_param_get(req, "id")));

	return database_select(sql, (void *)list, callback) && list->size;
}

bool pwdman_count_by_id(struct request *req)
{
	List *temp_list = list_new(sizeof(struct pwdman), NULL);

	pwdman_find_by_id(req, temp_list);

	if (list_isempty(temp_list) == 1) {
		fprintf(stderr, "list_isempty check\n" );
		return false;
	}

	return true;
}

List *pwdman_print_all(void)
{
	struct pwdman *temp;
	List *temp_list = list_new(sizeof(struct pwdman), destroyer);
	pwdman_get_list(temp_list);

	if (temp_list == NULL) {
		fprintf(stderr, "List is empty\n");
		return NULL;
	}

	return temp_list;
}

static bool pwdman_storage_add(struct pwdman *temp)
{
	char sql[BUFFSIZE];
	const char sql_buf[] = "INSERT INTO " INFO_TABLE " (site, email, password) VALUES('%s', '%s', '%s')";

	sprintf(sql, sql_buf, temp->site, temp->email, temp->password);

	if (!database_cud(sql)) {
		printf("ERROR\n");
		return false;
	}

	return true;

}

static bool pwdman_storage_update(struct pwdman *temp)
{
	char sql[BUFFSIZE];
	const char sql_buf[] = "UPDATE " INFO_TABLE " SET (site, email, password) = ('%s', '%s', '%s') WHERE id = '%d'";

	sprintf(sql, sql_buf, temp->site, temp->email, temp->password, temp->id);

	if (!database_cud(sql)) {
		printf("ERROR\n");
		return false;
	}

	return true;

}

static void pwdman_prepare_struct(struct request *req, struct pwdman *temp, int state)
{
	switch(state) {

		case PWDMAN_ADD :
			temp->site = request_param_get(req, "site");
			temp->email = request_param_get(req, "email");
			temp->password = request_param_get(req, "password");
			break;

		case PWDMAN_UPDATE :
			temp->id = atoi(request_param_get(req, "id"));
			temp->site = request_param_get(req, "site");
			temp->email = request_param_get(req, "email");
			temp->password = request_param_get(req, "password");
			break;
	}

}

static bool pwdman_get_list(List *list)
{
	const char sql[] = "SELECT * FROM " INFO_TABLE ;

	return database_select(sql, (void *)list, callback) && list->size;
}

static void destroyer(void *data)
{
	free(((struct pwdman *)data)->site);
	free(((struct pwdman *)data)->email);
	free(((struct pwdman *)data)->password);

	free(data);
}

int callback(void *list, int count, char **data, char **columns)
{
    int idx;
	struct pwdman temp = {0};

    temp.id = atoi(data[0]);
    temp.site = strdup(data[1]);
    temp.email = strdup(data[2]);
    temp.password = strdup(data[3]);

    list_push((List *)list, (void *)&temp);

    return 0;
}