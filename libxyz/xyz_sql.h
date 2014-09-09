
#ifndef __XYZ_SQL_H__
#define __XYZ_SQL_H__

#ifdef USE_MYSQL

struct xyz_mysql_t;

struct xyz_mysql_t *xyz_mysql_connect(char *host, int port, char *user, char *passwd, char *database);
void xyz_mysql_close(struct xyz_mysql_t *mysql);
int xyz_mysql_exec(struct xyz_mysql_t *mysql, int flag, char *sqlstring);
int xyz_mysql_fetchrow(struct xyz_mysql_t *mysql);
char *xyz_mysql_getfield(struct xyz_mysql_t *mysql, int idx);
void xyz_mysql_execend(struct xyz_mysql_t *mysql);

#endif // USE_MYSQL
#ifdef USE_PGSQL

struct xyz_pgsql_t;

struct xyz_pgsql_t *xyz_pgsql_connect(char *host, int port, char *user, char *passwd, char *database);
void xyz_pgsql_close(struct xyz_pgsql_t *pgsql);
int xyz_pgsql_exec(struct xyz_pgsql_t *pgsql, int flag, char *sqlstring);
char *xyz_pgsql_fetchfield(struct xyz_pgsql_t *pgsql, int row, int col);
void xyz_pgsql_execend(struct xyz_pgsql_t *pgsql);

#endif // USE_PGSQL
#ifdef USE_SQLITE3

struct xyz_sqlite3_t;

typedef int (*xyz_sqlite3_cb)(void *userdata, int argc, char **argv, char **colname);
struct xyz_sqlite3_t *xyz_sqlite3_open(char *filename);
void xyz_sqlite3_close(struct xyz_sqlite3_t *sqlite3); 
int xyz_sqlite3_exec(struct xyz_sqlite3_t *sqlite3, char *sql, xyz_sqlite3_cb cb, void *userdata);

#endif // USE_SQLITE3

#endif // __XYZ_SQL_H__

