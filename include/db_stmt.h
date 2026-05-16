#ifndef __DB_STMT_H
#define __DB_STMT_H

#include <stdbool.h>
#include <sqlite3.h>
#include <list.h>

typedef int (*db_row_cb)(void *, sqlite3_stmt *);

bool db_exec_stmt(const char *sql, const char *types, ...);
bool db_query_stmt(const char *sql, void *data, db_row_cb cb, const char *types, ...);

#endif