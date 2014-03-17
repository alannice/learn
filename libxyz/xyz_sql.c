/**
 * mysql pgsql
 * cc xyz_sql.c `mysql_config --cflags` `mysql_config --libs` -I`pg_config --includedir` -L`pg_config --libdir` -lpq
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xyz_sql.h"

/// MYSQL

/// mysql_config --cflags
/// mysql_config --libs
/// -lmysqlclient 

#include <mysql/mysql.h>

#define XYZ_MYSQL_CHARSET      "utf8"
#define XYZ_MYSQL_TIMEOUT      (8)

struct xyz_mysql_t {
    MYSQL *mysql;
    MYSQL_RES *mysql_res;
    MYSQL_ROW mysql_row;
};

struct xyz_mysql_t *xyz_mysql_connect(char *host, int port, char *user, char *passwd, char *database)
{
    struct xyz_mysql_t *mysql;

    mysql = malloc(sizeof(struct xyz_mysql_t));
    if(mysql == NULL) {
        return NULL;
    }

    if((mysql->mysql = mysql_init(NULL)) == NULL) {
        free(mysql);
        return NULL;
    }

    int timeout = XYZ_MYSQL_TIMEOUT;
    if (mysql_options(mysql->mysql, MYSQL_OPT_CONNECT_TIMEOUT, (const char *)(&timeout)) != 0) {
        mysql_close(mysql->mysql);
        free(mysql);
        return NULL;
    }

    if (mysql_options(mysql->mysql, MYSQL_SET_CHARSET_NAME, XYZ_MYSQL_CHARSET) != 0) {
        mysql_close(mysql->mysql);
        free(mysql);
        return NULL;
    }

    if (mysql_real_connect(mysql->mysql, host, user, passwd, database, port, NULL, 0) == NULL) {
        fprintf(stderr,"my conn error :%s\n", mysql_error(mysql->mysql));
        mysql_close(mysql->mysql);
        free(mysql);
        return NULL;
    }

    return mysql;
}

void xyz_mysql_close(struct xyz_mysql_t *mysql)
{
    if(mysql) {
        if(mysql->mysql_res) {
            mysql_free_result(mysql->mysql_res);
        }
        if(mysql->mysql) {
            mysql_close(mysql->mysql);
        } 
        
        free(mysql);
    }

    return;
}

int xyz_mysql_exec(struct xyz_mysql_t *mysql, int flag, char *sqlstring)
{
    char execstring[512] = {0};

    if(mysql == NULL || sqlstring == NULL) {
        return -1;
    }

    int len = mysql_real_escape_string(mysql->mysql, execstring, sqlstring, strlen(sqlstring)); 

    if(mysql_real_query(mysql->mysql, execstring, len) != 0) {
        fprintf(stderr,"my query error :%s\n", mysql_error(mysql->mysql));
        return -1;
    }

    // 是否保留结果集.
    if(flag == 0) {
        return 0;
    }

    if (((mysql->mysql_res = mysql_store_result(mysql->mysql)) == NULL)) {
        fprintf(stderr,"my store error :%s\n", mysql_error(mysql->mysql));
        return -1;
    }

    return mysql_num_rows(mysql->mysql_res); 
}

int xyz_mysql_fetchrow(struct xyz_mysql_t *mysql)
{
    if(mysql == NULL || mysql->mysql_res == NULL) {
        return -1;
    }

    mysql->mysql_row = mysql_fetch_row(mysql->mysql_res); 
    if(mysql->mysql_row == NULL) {
        fprintf(stderr,"my fetch error :%s\n", mysql_error(mysql->mysql));
        return -1;
    }

    return 0;
}

char *xyz_mysql_getfield(struct xyz_mysql_t *mysql, int idx)
{
    if(mysql == NULL || mysql->mysql_row == NULL) {
        return NULL;
    }

    return mysql->mysql_row[idx];
}

void xyz_mysql_execend(struct xyz_mysql_t *mysql)
{
    if(mysql == NULL || mysql->mysql_res == NULL) {
        return;
    }

    mysql_free_result(mysql->mysql_res);
    mysql->mysql_res = NULL; 

    return;
}

//////////////////////////////////////////////////////////////////////////////

// PGSQL

/// pg_config --includedir
/// pg_config --libdir
/// -lpq

#include "libpq-fe.h"

struct xyz_pgsql_t {
    PGconn *pg_connect;
    PGresult *pg_result;
};

struct xyz_pgsql_t *xyz_pgsql_connect(char *host, int port, char *user, char *passwd, char *database)
{
    char connstring[256] = {0};
    struct xyz_pgsql_t *pgsql = malloc(sizeof(struct xyz_pgsql_t));
    if(pgsql == NULL) {
        return NULL;
    }

    snprintf(connstring, sizeof(connstring)-1,
            "host=%s port=%d dbname=%s user=%s password=%s",
            host, port, database, user, passwd);

    pgsql->pg_connect = PQconnectdb(connstring);
    if(PQstatus(pgsql->pg_connect) != CONNECTION_OK) {
        fprintf(stderr,"pg conn error :%s\n", PQerrorMessage(pgsql->pg_connect));
        PQfinish(pgsql->pg_connect);
        return NULL;
    }

    return pgsql;
}

void xyz_pgsql_close(struct xyz_pgsql_t *pgsql)
{
    if(pgsql) {
        if(pgsql->pg_result) PQclear(pgsql->pg_result);

        if(pgsql->pg_connect) PQfinish(pgsql->pg_connect);

        free(pgsql);
    }

    return;
}

int xyz_pgsql_exec(struct xyz_pgsql_t *pgsql, int flag, char *sqlstring)
{
    char execstring[512] = {0};

    if(pgsql == NULL || sqlstring == NULL) {
        return -1;
    }

    PQescapeString(execstring, sqlstring, strlen(sqlstring));

    pgsql->pg_result = PQexec(pgsql->pg_connect, execstring);
    if(PQresultStatus(pgsql->pg_result) != PGRES_TUPLES_OK) {
        fprintf(stderr,"pg exec error :%s\n", PQerrorMessage(pgsql->pg_connect));
        PQclear(pgsql->pg_result);
        pgsql->pg_result = NULL;
        return -1;
    }

    if(flag == 0) {
        PQclear(pgsql->pg_result);
        pgsql->pg_result = NULL;
        return 0;
    }

    return PQntuples(pgsql->pg_result);
}

char *xyz_pgsql_fetchfield(struct xyz_pgsql_t *pgsql, int row, int col)
{
    if(pgsql == NULL && pgsql->pg_result == NULL) {
        return NULL;
    }

    return PQgetvalue(pgsql->pg_result, row, col);
}

void xyz_pgsql_execend(struct xyz_pgsql_t *pgsql)
{
    if(pgsql == NULL || pgsql->pg_result == NULL) {
        return;
    }

    PQclear(pgsql->pg_result);
    pgsql->pg_result = NULL;

    return;
}

// end.

#if 0

int main(void)
{
    struct xyz_mysql_t *mysql = xyz_mysql_connect("localhost", 3306, "yuan", "yuanzc", "test");
    xyz_mysql_exec(mysql, 0, "select * from test");
    xyz_mysql_close(mysql);

    struct xyz_pgsql_t *pgsql = xyz_pgsql_connect("localhost", 5432, "yuan", "yuanzc", "test");
    xyz_pgsql_exec(pgsql, 0, "select * from test");
    xyz_pgsql_close(pgsql);

    return 0;
}

#endif // 
