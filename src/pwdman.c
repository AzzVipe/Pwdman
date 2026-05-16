#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iter.h>
#include <crypto.h>
#include <pwdman.h>
#include <db_stmt.h>
#include <database.h>
#include <command.h>
#include <pwdman_response.h>

static bool pwdman_storage_add(struct pwdman *temp);
static bool pwdman_storage_update(struct pwdman *temp);
static bool pwdman_get_list(List *list);
static void pwdman_prepare_struct(struct request *req, struct pwdman *temp, int state);
static void destroyer(void *data);
static int pwdman_row_cb(void *list, sqlite3_stmt *stmt);

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
	if (commands[index].req_handle(&col) == -1) {
		fprintf(stderr, "request_handle : Error\n");
		pwdman_response_handle_error(&col);
		return;
	}

	// responses
	if (commands[index].res_handle(&col) == -1) {
		fprintf(stderr, "response_handle : Error\n");
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
	int id = atoi(request_param_get(req, "id"));

	return db_exec_stmt("DELETE FROM " INFO_TABLE " WHERE id=?", "i", id);
}

bool pwdman_find_by_email(struct request *req, List *list)
{
	const char *email = request_param_get(req, "email");
	char pattern[256];

	snprintf(pattern, sizeof(pattern), "%%%s%%", email);

	return db_query_stmt("SELECT * FROM " INFO_TABLE " WHERE email LIKE ?", 
		list, pwdman_row_cb, "s", pattern
	) && list->size > 0;
}

bool pwdman_find_by_site(struct request *req, List *list)
{
	const char *site = request_param_get(req, "site");
	char pattern[256];
	
	snprintf(pattern, sizeof(pattern), "%%%s%%", site);
	
	return db_query_stmt("SELECT * FROM " INFO_TABLE " WHERE site LIKE ?",
		list, pwdman_row_cb, "s", pattern
	) && list->size > 0;
}

bool pwdman_find_by_id(struct request *req, List *list)
{
	int id = atoi(request_param_get(req, "id"));

	return db_query_stmt("SELECT * FROM " INFO_TABLE " WHERE id=?",
		list, pwdman_row_cb, "i", id
	) && list->size > 0;
}

bool pwdman_count_by_id(struct request *req)
{
	List *temp_list = list_new(sizeof(struct pwdman), NULL);
	pwdman_find_by_id(req, temp_list);

	bool found = !list_isempty(temp_list);

	list_destroy(&temp_list);

	return found;
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
	char *ct, *iv, *tag;

	if (!crypto_encrypt(temp->password, &ct, &iv, &tag))
		return false;

	bool ok = db_exec_stmt(
		"INSERT INTO " INFO_TABLE " (site, email, password, iv, tag) VALUES(?,?,?,?,?)",
		"sssss", temp->site, temp->email, ct, iv, tag);

	free(ct);
	free(iv);
	free(tag);

	return ok;
}

static bool pwdman_storage_update(struct pwdman *temp)
{
	char *ct, *iv, *tag;

	if (!crypto_encrypt(temp->password, &ct, &iv, &tag))
		return false;

	bool ok = db_exec_stmt(
		"UPDATE " INFO_TABLE " SET site=?, email=?, password=?, iv=?, tag=? WHERE id=?",
		"sssssi", temp->site, temp->email, ct, iv, tag, temp->id);

	free(ct);
	free(iv);
	free(tag);

	return ok;
}

static void pwdman_prepare_struct(struct request *req, struct pwdman *temp, int state)
{
	switch (state) {
		case PWDMAN_ADD:
			temp->site = request_param_get(req, "site");
			temp->email = request_param_get(req, "email");
			temp->password = request_param_get(req, "password");

			break;

		case PWDMAN_UPDATE:
			temp->id = atoi(request_param_get(req, "id"));
			temp->site = request_param_get(req, "site");
			temp->email = request_param_get(req, "email");
			temp->password = request_param_get(req, "password");

			break;
	}
}

static bool pwdman_get_list(List *list)
{
	return db_query_stmt("SELECT * FROM " INFO_TABLE, list, pwdman_row_cb, NULL);
}

// Callback
static int pwdman_row_cb(void *list, sqlite3_stmt *stmt)
{
	struct pwdman temp = {0};
	char *plaintext = NULL;

	temp.id = sqlite3_column_int(stmt, 0);
	temp.site = strdup((char *)sqlite3_column_text(stmt, 1));
	temp.email = strdup((char *)sqlite3_column_text(stmt, 2));

	const char *ct = (char *)sqlite3_column_text(stmt, 3);
	const char *iv = (char *)sqlite3_column_text(stmt, 4);
	const char *tag = (char *)sqlite3_column_text(stmt, 5);

	if (!crypto_decrypt(ct, iv, tag, &plaintext))
		temp.password = strdup("[decrypt failed]");
	else
		temp.password = plaintext;

	list_push((List *)list, &temp);

	return 0;
}

static void destroyer(void *data)
{
	free(((struct pwdman *)data)->site);
	free(((struct pwdman *)data)->email);
	free(((struct pwdman *)data)->password);

	free(data);
}