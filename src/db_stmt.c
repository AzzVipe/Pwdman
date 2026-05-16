#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <db_stmt.h>
#include <database.h>

static bool bind_params(sqlite3_stmt *stmt, const char *types, va_list ap)
{
  if (!types)
    return true;

  for (int i = 0; types[i]; i++) {
    int col = i + 1;

    switch (types[i]) {
      case 's': {
        const char *val = va_arg(ap, const char *);
        if (sqlite3_bind_text(stmt, col, val, -1, SQLITE_TRANSIENT) != SQLITE_OK)
          return false;
        break;
      }
      case 'i': {
        int val = va_arg(ap, int);
        if (sqlite3_bind_int(stmt, col, val) != SQLITE_OK)
          return false;
        break;
      }
      default:
        fprintf(stderr, "db_stmt: unknown type '%c'\n", types[i]);
        return false;
    }
  }

  return true;
}

bool db_exec_stmt(const char *sql, const char *types, ...)
{
  sqlite3 *conn = database_get_conn();
  sqlite3_stmt *stmt = NULL;

  if (sqlite3_prepare_v2(conn, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "db_exec_stmt prepare: %s\n", sqlite3_errmsg(conn));
    return false;
  }

  va_list ap;
  va_start(ap, types);
  bool ok = bind_params(stmt, types, ap);
  va_end(ap);

  if (!ok || sqlite3_step(stmt) != SQLITE_DONE) {
    fprintf(stderr, "db_exec_stmt step: %s\n", sqlite3_errmsg(conn));
    sqlite3_finalize(stmt);
    return false;
  }

  sqlite3_finalize(stmt);

  return true;
}

bool db_query_stmt(const char *sql, void *data, db_row_cb cb, const char *types, ...)
{
  sqlite3 *conn = database_get_conn();
  sqlite3_stmt *stmt = NULL;

  if (sqlite3_prepare_v2(conn, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "db_query_stmt prepare: %s\n", sqlite3_errmsg(conn));
    return false;
  }

  va_list ap;
  va_start(ap, types);
  bool ok = bind_params(stmt, types, ap);
  va_end(ap);

  if (!ok)
  {
    sqlite3_finalize(stmt);
    return false;
  }

  int rc;
  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    cb(data, stmt);

  sqlite3_finalize(stmt);

  return rc == SQLITE_DONE;
}