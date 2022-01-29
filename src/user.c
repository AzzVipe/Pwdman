#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <user.h>
#include <iter.h>
#include <database.h>
#include <command.h>

struct user user = {0};

static bool user_add(struct user *temp);
static bool user_update(struct user *temp);

void user_init(void)
{
	char username[32], password[32];

	printf("Enter Username : ");
	fgets(username, 32, stdin);

	printf("Enter Passsword : ");
	fgets(password, 32, stdin);

	user.username = strdup(username);
	user.password = strdup(password);

	user_add(&user);
}

static bool user_add(struct user *temp)
{
	char sql[BUFFSIZE];
	const char sql_buf[] = "INSERT INTO " USER_TABLE " (username, password) VALUES('%s', '%s')";

	sprintf(sql, sql_buf, temp->username, temp->password);

	if (!database_cud(sql)) {
		printf("ERROR\n");
		return false;
	}

	return true;
}

static bool user_update(struct user *temp)
{
	char sql[BUFFSIZE];
	const char sql_buf[] = "UPDATE " USER_TABLE " SET (username, password) = ('%s', '%s') WHERE id = '%d'";

	sprintf(sql, sql_buf, temp->username, temp->password, temp->id);

	if (!database_cud(sql)) {
		printf("ERROR\n");
		return false;
	}

	return true;
}

static bool user_delete(struct request *req)
{
	int id;
	char sql[BUFFSIZE];
	const char sql_buf[] = "DELETE FROM " USER_TABLE " WHERE id = %d";

	id = atoi(request_param_get(req, "id"));
	sprintf(sql, sql_buf, id);

	return database_cud(sql);
}

