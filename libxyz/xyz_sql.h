
#ifndef __XYZ_SQL_H__
#define __XYZ_SQL_H__

struct xyz_mysql_t;

struct xyz_mysql_t *xyz_mysql_connect(char *host, int port, char *user, char *passwd, char *database);
void xyz_mysql_close(struct xyz_mysql_t *mysql);
int xyz_mysql_exec(struct xyz_mysql_t *mysql, int flag, char *sqlstring);
int xyz_mysql_fetchrow(struct xyz_mysql_t *mysql);
char *xyz_mysql_getfield(struct xyz_mysql_t *mysql, int idx);
void xyz_mysql_fetchend(struct xyz_mysql_t *mysql);

struct xyz_pgsql_t;

struct xyz_pgsql_t *xyz_pgsql_connect(char *host, int port, char *user, char *passwd, char *database);
void xyz_pgsql_close(struct xyz_pgsql_t *pgsql);
int xyz_pgsql_exec(struct xyz_pgsql_t *pgsql, int flag, char *sqlstring);
char *xyz_pgsql_fetchfield(struct xyz_pgsql_t *pgsql, int row, int col);
void xyz_pgsql_fetchend(struct xyz_pgsql_t *pgsql);

#endif // __XYZ_SQL_H__

