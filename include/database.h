#ifndef __DATABASE_H
#define __DATABASE_H

#include <stdbool.h>
#include <sqlite3.h>

extern const char user_table[];
extern const char info_table[];

typedef int (*database_callback)(void*,int,char**, char**);

void  database_create_app(void);
char *database_get_name(void);

bool database_exec(const char *sql);

bool database_cud(const char *sql, ...);
bool database_select(const char *sql, void *data, database_callback cb, ...);

sqlite3 *database_get_conn(void);  

#endif