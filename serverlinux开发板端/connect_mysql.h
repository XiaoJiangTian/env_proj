#ifndef __CONNECT_MYSQL_H
#define __CONNECT_MYSQL_H

#include<mysql/mysql.h>

MYSQL * c_db_init();

char * c_db_get(MYSQL *mysql,char *sql);
int c_db_insert(MYSQL *mysql, char *sql);
int c_db_select(MYSQL *mysql,char *sql,char *name,char *account,char *password);
int c_db_update(MYSQL *mysql,char *sql);
int c_db_delete(MYSQL *mysql,char *sql);

#endif