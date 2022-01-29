#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <database.h>
#include <sqlite3.h>

#define DB_NAME ".pwdman.db"

const char info_table[] = "CREATE TABLE info(   \
	id INTEGER PRIMARY KEY AUTOINCREMENT,       \
	site VARCHAR(32),                           \
	email VARCHAR(32),                          \
	password VARCHAR(255)                       \
)";

const char user_table[] = "CREATE TABLE user(   \
	id INTEGER PRIMARY KEY AUTOINCREMENT,       \
	username VARCHAR(64),                           \
	password VARCHAR(255)                       \
)";


static sqlite3 *database_get_conn();

char *database_get_name(void)
{
	return DB_NAME;
}

void database_create_app(void)
{
	sqlite3 *conn = database_get_conn();

	sqlite3_exec(conn, user_table, NULL, NULL, NULL);
	sqlite3_exec(conn, info_table, NULL, NULL, NULL);
}


bool database_cud(const char *buf)
{
	char *err_buf;

	sqlite3 *conn = database_get_conn();

	if(sqlite3_exec(conn, buf, NULL, NULL, &err_buf) != SQLITE_OK) {
		fprintf(stderr, "%s\n", err_buf);
		sqlite3_free(err_buf);
		return false;
	}

	return true;
}

bool database_select(const char *buf, void *list, database_callback xcallback)
{
	char *err_buf;
	sqlite3 *conn = database_get_conn();

	if(sqlite3_exec(conn, buf, xcallback, list, &err_buf) != SQLITE_OK) {
		fprintf(stderr, "%s\n", err_buf);
		sqlite3_free(err_buf);
		return false;
	}

	return true;
}

static sqlite3 *database_get_conn()
{
	static sqlite3 *conn = NULL;
	if (conn == NULL)
	{
		if (sqlite3_open(DB_NAME, &conn) != SQLITE_OK)
		{
			fprintf(stderr, "sqlite3_open(): failed to open database: (%d: %s)\n", sqlite3_errcode(conn), sqlite3_errmsg(conn));
			exit(1);
		}
	}
	return conn;
}
