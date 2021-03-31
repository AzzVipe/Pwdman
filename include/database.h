#ifndef __DATABASE_H
#define __DATABASE_H

extern const char info_table[];

typedef int (*database_callback)(void*,int,char**, char**);

void database_create_app(void);
bool database_cud(const char *buf);
int  database_is_exist(char *file);
bool database_select(const char *buf, void *list, database_callback xcallback);


#endif
