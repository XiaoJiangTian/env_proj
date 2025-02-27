 //注意包含正确性
#include<stdio.h>
#include<string.h>
#include "connect_mysql.h"
#define C_DB_SERVER_IP   "127.0.0.1"
#define C_DB_SERVER_PORT 3306
#define C_DB_USERNAME    "root"
#define C_DB_PASSWORD    "root"
#define C_DB_DEFAULT_DB  "person_info"

//插入
int c_db_insert(MYSQL *mysql, char *sql)
{
    if(mysql == NULL)
    {
        printf("%s",mysql_error(mysql));
        return -1;
    }
    if(mysql_real_query(mysql,sql,strlen(sql)))
    {
        printf("mysql_real_query : %s\n",mysql_error(mysql));
        return -2;
    }
    return 0;
}

//查询
int c_db_select(MYSQL *mysql,char *sql,char *name,char *account,char *password)
{
    if(mysql == NULL)
    {
        printf("%s",mysql_error(mysql));
        return -1;
    }
    //mysql_real_query是操作的关键 参数分别是 mysql管道,sql语句,sql语句长度
    //mysql_real_query 返回0表示成功
    if(mysql_real_query(mysql,sql,strlen(sql)))
    {
        printf("mysql_real_query : %s\n",mysql_error(mysql));
        return -2;
    }

    //把查到的数据存储在内存中
    //mysql_store_result(mysql)会返回一个MYSQL_RES类型的指针，存储查询sql语句运行的结果
    MYSQL_RES *res = mysql_store_result(mysql);
    if(res == NULL)
    {
        printf("mysql_store_result : %s",mysql_error(mysql));
        return -3;
    }
    //获取列的个数
    int rows = mysql_num_rows(res);
    //获取行的个数
    int fields = mysql_num_fields(res);
    //(*row_in) = rows;
    //printf("rows = %d fields = %d\n", rows, fields);

    //把结果打印
    //mysql_fetch_row() 返回一个查询结果集的数组

    MYSQL_ROW row;
    while((row = mysql_fetch_row(res)))//行
    {
        // for (int i = 0; i < fields; i++)//列
        // {
        //     printf("%s\t",row[i]);
        // }


        // if(account!= NULL && password != NULL && strcmp(row[2],account)==0 && strcmp(row[3],password)==0)
        // {
        //     if(name != NULL && strcmp(row[0],name)==0)
        //     {
        //         return 2;
        //     }
        //     else if(name != NULL &&strcmp(row[0],name)!=0)
        //     {
        //         continue;
        //     }
        //     return 1;
        // }
        // else if(strcmp(row[0],name)==0)
        // {
        //     return 2;
        // }
        // else
        // {
        //     continue;
        // }

        if(name!=NULL && account!=NULL && password!=NULL)
        {
            if(strcmp(name,row[0])==0 &&strcmp(account,row[2])==0 && strcmp(password,row[3])==0)
            {
                return 1;
            }
        }
        else if(name!=NULL && account==NULL && password==NULL)
        {
            if(strcmp(name,row[0])==0)
            {
                return 2;//这种情况下返回2是查找对应项是否存在
            }
        }
        //printf("\n");
    }
    //printf("no such user\n");
    return 0;
}


char *c_db_get(MYSQL *mysql,char *sql)
{
    if(mysql == NULL)
    {
        printf("%s",mysql_error(mysql));
        return NULL;
    }
    //mysql_real_query是操作的关键 参数分别是 mysql管道,sql语句,sql语句长度
    //mysql_real_query 返回0表示成功
    if(mysql_real_query(mysql,sql,strlen(sql)))
    {
        printf("mysql_real_query : %s\n",mysql_error(mysql));
        return NULL;
    }

    //把查到的数据存储在内存中
    //mysql_store_result(mysql)会返回一个MYSQL_RES类型的指针，存储查询sql语句运行的结果
    MYSQL_RES *res = mysql_store_result(mysql);
    if(res == NULL)
    {
        printf("mysql_store_result : %s",mysql_error(mysql));
        return NULL;
    }
    //获取列的个数
    int rows = mysql_num_rows(res);
    //获取行的个数
    int fields = mysql_num_fields(res);

    //printf("rows = %d fields = %d\n", rows, fields);

    //把结果打印
    //mysql_fetch_row() 返回一个查询结果集的数组
    MYSQL_ROW row;
    char *info = (char *)malloc(rows*fields*(20+4));
    memset(info,0,rows*fields*(20+4));

    char *info1 = (char *)malloc(fields*(20+4));
    

    while((row = mysql_fetch_row(res)))//行
    {
        //memset(info,0,fields*(20+4));
        sprintf(info1,":%s:%d:%s:%s",row[0],atoi(row[1]),row[2],row[3]);
        strcat(info,info1);
        // for (int i = 0; i < fields; i++)//列
        // {
        //     strcat(info,row[i]);
        //     printf("%s\t",row[i]);
        // }
        // printf("\n");
    }
    //printf("%s\n",info);
    
    return info;
}

//更新
int c_db_update(MYSQL *mysql,char *sql)
{
    if(mysql == NULL)
    {
        printf("%s",mysql_error(mysql));
        return -1;
    }
    //mysql_real_query是操作的关键 参数分别是 mysql管道,sql语句,sql语句长度
    //mysql_real_query 返回0表示成功
    if(mysql_real_query(mysql,sql,strlen(sql)))
    {
        printf("mysql_real_query : %s\n",mysql_error(mysql));
        return -2;
    }
    return 0;
}

//删除
int c_db_delete(MYSQL *mysql,char *sql)
{
        if(mysql == NULL)
    {
        printf("%s",mysql_error(mysql));
        return -1;
    }
    //mysql_real_query 是操作的关键 参数分别是 mysql管道,sql语句,sql语句长度
    //mysql_real_query 返回0表示成功
    if(mysql_real_query(mysql,sql,strlen(sql)))
    {
        printf("mysql_real_query : %s\n",mysql_error(mysql));
        return -2;
    }
    return 0;
}

MYSQL *c_db_init()
{
    MYSQL *mysql=malloc(sizeof(MYSQL));
    if(mysql_init(mysql) == NULL)
    {
        printf("mysql_init : %s\n",mysql_error(mysql));
        return NULL;
    }

    // mysql_real_connect 若返回0表示则表示连接失败
    if(!mysql_real_connect(mysql,
	C_DB_SERVER_IP, C_DB_USERNAME, 
    C_DB_PASSWORD, C_DB_DEFAULT_DB, 
    C_DB_SERVER_PORT,NULL, 0))
    {
        printf("mysql_real_connect : %s\n",mysql_error(mysql));
        return NULL;
    }
    return mysql;
}

// int main()
// {
// 	MYSQL mysql;
//     if(mysql_init(&mysql) == NULL)
//     {
//         printf("mysql_init : %s\n",mysql_error(&mysql));
//         return -1;
//     }

//     // mysql_real_connect 若返回0表示则表示连接失败
//     if(!mysql_real_connect(&mysql,
// 	C_DB_SERVER_IP, C_DB_USERNAME, 
//     C_DB_PASSWORD, C_DB_DEFAULT_DB, 
//     C_DB_SERVER_PORT,NULL, 0))
//     {
//         printf("mysql_real_connect : %s\n",mysql_error(&mysql));
//         return -2;
//     } 

//     //添加操作
//     // char insert_sql[] = "INSERT login_info(name,id,account,password) VALUES('test',2,'123','123')";
//     // c_db_insert(&mysql,insert_sql);

//     //查询操作
//     char select_sql[] = "SELECT * FROM login_info";
//     c_db_select(&mysql, select_sql);

//     //更新操作
//     char update_sql[] = "UPDATE login_info SET NAME='ceshi' WHERE id='2'";
//     c_db_update(&mysql, update_sql);

//     //删除操作
//     char delete_sql[] = "delete from login_info where id = '2'";
//     c_db_delete(&mysql, delete_sql);

//     return 0;
// }


//编译：gcc connect_mysql.c -o connect_mysql -L/user/lib/mysql -lmysqlclient 