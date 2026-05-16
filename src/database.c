#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include <database.h>
#include <sqlite3.h>

#define DB_NAME ".pwdman.db"

const char info_table[] = "CREATE TABLE info(      \
	id INTEGER PRIMARY KEY AUTOINCREMENT,           \
	site VARCHAR(32),                               \
	email VARCHAR(32),                              \
	password BLOB,                                  \
	iv BLOB,                                        \
	tag BLOB                                        \
)";

sqlite3 *database_get_conn(void)
{
	static sqlite3 *conn = NULL;

	if (conn == NULL) {
		if (sqlite3_open(DB_NAME, &conn) != SQLITE_OK) {
			fprintf(stderr, "sqlite3_open failed: %s\n", sqlite3_errmsg(conn));
			exit(1);
		}
	}

	return conn;
}

char *database_get_name(void) 
{ 
	return DB_NAME; 
}

void database_create_app(void)
{
	sqlite3 *conn = database_get_conn();
	sqlite3_exec(conn, info_table, NULL, NULL, NULL);
}

// Only for DDL (CREATE TABLE etc.) — never call with user input
bool database_exec(const char *sql)
{
	char *err = NULL;
	if (sqlite3_exec(database_get_conn(), sql, NULL, NULL, &err) != SQLITE_OK) {
		fprintf(stderr, "database_exec: %s\n", err);
		sqlite3_free(err);
		return false;
	}

	return true;
}