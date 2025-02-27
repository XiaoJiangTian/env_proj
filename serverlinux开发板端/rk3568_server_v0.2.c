#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<ctype.h>
#include<string.h>


#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/wait.h>
#include<signal.h>
#include<pthread.h>

#include "threadpool.h"
#include "connect_mysql.h"


#include "../device-core/aiot_device_api.h"
#include "demo_config.h"

#define IP "192.168.184.4"
#define PORT "9999"
#define EVENTSSIZE 1024//epoll能承受的连接

#define BUFFER_SIZE 2000
#define MEMSIZE 1024

#define MIN_THR 5
#define MAX_THR 1024
#define MAX_QUEUE 1024


// 声明条件变量和互斥锁，用于主线程与其他线程同步
// pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
// pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int check_state(char *temp);

void *init_rk_to_aliyun();
char *get_info_tag(char *info);
int get_server_state(char *info);

void case1_login_process(char *info,MYSQL *mysql,int talkfd);
void case1_register_process(char *info,MYSQL *mysql,int talkfd);
void case1_goverment_login_process(MYSQL *mysql,int talkfd);
void case1_goverment_change_process(char *info,MYSQL *mysql,int talkfd);
void case1_goverment_delete_process(char *info,MYSQL *mysql,int talkfd);

void case2_request_data_process(int talkfd,char *ptr);
void case2_set_data_process(char *info,void *device_client);

void cloud_info_trans_process(char *buffer,FILE *fp,int shmid,int shmid_1);

int tcp_init(int *linkfd,int *val,struct sockaddr_in* server_st,int *epfd,struct epoll_event *tmp);

static void sys_err(char *s)
{
    perror(s);
    exit(1);
}
struct thread_info_st
{
    //int *ret;
    //添加还需的变量
    //uint8_t server_state;
    //int *pipefd;
    //char *buffer;
    //struct epoll_event* epoll_event;

    int talkfd;
    int epfd;
    struct epoll_event tmp;
    void *device_client;
    char *ptr;
    char *ptr_1;
    MYSQL *mysql;  
};



//服务器针对每个客户端的处理程序
//放到线程池里的话这里要筛选一下，将前面的talkfd记录一下，并且每个线程只对其对应对话的fd进行反应
void fun_process_client(void *arg)
{
    struct thread_info_st *tmp_thread_info_st = (struct thread_info_st *)arg;

    //重要，存放发送请求的客户端信息
    //struct epoll_event *events = tmp_thread_info_st->epoll_event;
    //int *ret = tmp_thread_info_st->ret;
    //uint8_t server_state = tmp_thread_info_st->server_state;
    //int *pipefd = tmp_thread_info_st->pipefd;
    //char *buffer = tmp_thread_info_st->buffer;

    int talkfd = tmp_thread_info_st->talkfd;
    struct epoll_event tmp = tmp_thread_info_st->tmp;
    int epfd = tmp_thread_info_st->epfd;
    MYSQL *mysql = tmp_thread_info_st->mysql;
    void *device_client = tmp_thread_info_st->device_client;
    char *ptr = tmp_thread_info_st->ptr;
    char *ptr_1 = tmp_thread_info_st->ptr_1;

    //共享内存和管道的问题需要解决
    char buffer[15];
    uint8_t server_state = 0;

    //talkfd是恒定的，event会变，传输进来变
    while(1)
    {
        //这里考虑阻塞等待ret更新
        char info[1024];//接收消息的缓冲区
        memset(info,0,1024);//每次使用接收缓冲区时，都需将其清空，防止之前数据的影响
        
        if(recv(talkfd,info,1024,0)==0) //有已连接客户端退出的场景
        {
            close(talkfd);
            //客户端退出才会使用
            if(epoll_ctl(epfd,EPOLL_CTL_DEL,talkfd,&tmp))
            {
                sys_err("epoll_ctl_thread:");
            }
            //connect_num--;
            //send(talkfd,"connect_num:%d",connect_num,0);
            //printf("client exit\n");
            
            //这里来个线程退出函数
            printf("client and the handle thread exit\n");
            return;
        }

        //要有一个输出信息来标识的
        printf("info:%s from thread:0x%x\n",info,(unsigned int)pthread_self()); //用于调试使用，打印已连接的各客户端发来的信息
        
        //如果存在状态切换场景就更改状态值
        server_state = get_server_state(info);

        switch (server_state)
        {
            //网络连接界面
            case 0:
            {
                printf("server in case 0\n");
            }
            break;
            //登录界面处理
            case 1:
            {
                printf("server in case 1\n");
                
                //获取尾部tag
                char *lasttoken = NULL;
                lasttoken = get_info_tag(info);
                
                //分情况处理
                //处理登录情况login
                //只会走其中一个分支，所以每个都需要重新定义变量
                if(strcmp(lasttoken,"login")==0)
                {
                    printf("case 1:login\n");
                    //处理登录相关的sql事务
                    case1_login_process(info,mysql,talkfd);
                }
                //注册情况
                else if(strcmp(lasttoken,"register")==0)
                {
                    printf("case 1:register\n");
                    case1_register_process(info,mysql,talkfd);
                    
                }
                //管理情况
                else if(strcmp(lasttoken,"goverment")==0)
                {
                    printf("case 1:goverment_login\n");
                    case1_goverment_login_process(mysql,talkfd);
                }
                //管理内容进行修改
                else if(strcmp(lasttoken,"goverment_change")==0)
                {
                    
                    printf("case 1:goverment_change\n");
                    case1_goverment_change_process(info,mysql,talkfd);

                }
                else if(strcmp(lasttoken,"goverment_delete")==0)
                {
                    printf("case 1:goverment_delete\n");
                    case1_goverment_delete_process(info,mysql,talkfd);
                }
                free(lasttoken);//防止内存泄漏，get_info_tag内申请的内存
                break;
            }
            break;

            //登录成功后的处理逻辑,处理请求数据
            case 2:
            {
                printf("server in case2\n");

                //获取管道数据开是否更新
                //close(pipefd[1]); // 关闭写端
                
                //这里管道的数据在多线程中会出问题，用的同一个，取走其他线程就没有了，无法使用
                //read(pipefd[0], buffer, sizeof(buffer)); // 读取管道中的数据
                strncpy(buffer,ptr_1,15);
                //printf("pipe received: %s\n", buffer);
                //获取数据,或者在接收到数据的时候，自动进行更新并且传输
                if(strcmp(info,"request_data")==0 || strcmp(buffer,"data_update")==0)
                {
                    //ptr没事，但pipefd这里会有问题
                    memset(buffer,0,15);
                    printf("case 2:request_data\n");
                    case2_request_data_process(talkfd,ptr);

                }
                else if(strncmp(info,"set_data:",9)==0)
                {
                    printf("case 2:set_data\n");
                    case2_set_data_process(info,device_client);
                }
            }
            break;
        } 
    }
    
}

char * extract_between(const char *str,char start_char,char end_char)
{
    char *start = strchr(str,start_char); //在字符串中找字符的函数
    if(start==NULL)
    {
        return NULL;
    }
    //printf("%s\n",start);
    //有就跳过当前找到的字符
    start++;

    char *end = strchr(start,end_char);
    if(end==NULL)
    {
        return NULL;
    }

    size_t len = end - start;
    //开辟新内存用于存储
    char *result = (char *)malloc(len+1);
    //printf("%s\n",result);
    if(result==NULL)
    {
        return NULL;
    }

    strncpy(result,start,len);
    result[len]='\0';
    return result;
}

pid_t pid = -1,pid_1=-1;

void handle_sigint(int sig) {
    if (pid > 0) {
        // 终止子进程
        kill(pid, SIGKILL); //发送信号
        printf("Child process 1 terminated\n");
    }
    if(pid_1>0)
    {
        kill(pid_1, SIGKILL);
        printf("Child process 2 terminated\n");
    }
    exit(0); // 终止主进程
}


//做出错判断
int main(int argc, char const *argv[])
{
    struct pool *ret_pool;

    //共享内存
    int shmid,shmid_1;
    char *ptr=NULL,*ptr_1=NULL;//共享内存区域

    shmid = shmget(IPC_PRIVATE,MEMSIZE,0664);//获取shmid
    if(shmid<0)
    {
        sys_err("fail to shmget:");
        // perror("fail to shmget\n");
        // exit(1);
    }

    //两个共享内存不同，上面用于传解析数据，下面用于通知数据更新
    shmid_1 = shmget(IPC_PRIVATE,MEMSIZE,0664);//获取shmid
    if(shmid_1<0)
    {
        sys_err("fail to shmget_1:");
        // perror("fail to shmget\n");
        // exit(1);
    }

   
    if(signal(SIGINT, handle_sigint) == SIG_ERR) //注册信号处理函数，当主线程SIGINT退出时，子线程也退出
    {
        sys_err("signal:");
        // perror("signal");
        // exit(1);
    }

    //注册一个信号处理函数，当服务器接收到信息后自动更新到客户端
    //signal(SIGUSR1, handle_custom_signal);

    pid = fork();//创建多进程
    if(pid == -1)
    {
        sys_err("fail to fork_1");
    }
    else if(pid==0)//子
    {
        pid_1 = fork();//第二次fork
        if(pid_1==-1)
        {
            sys_err("fail to fork_2");
        }
        else if(pid_1 == 0) //第二次的子
        {
            //驱动程序的应用层
        }
        else if(pid_1>0) //第二次的父
        {
            //aliyun接受和传送服务
            FILE *fp;
            char buffer[BUFFER_SIZE];
            char command[BUFFER_SIZE];

            // 构建要执行的命令
            snprintf(command, sizeof(command), "python3 test.py");

            // 执行命令并打开管道
            fp = popen(command, "r");
            if (fp == NULL) {
                sys_err("popen:");
                // perror("popen");
                // return 1;
            }
            // 读取命令的输出
            while (1) {
                cloud_info_trans_process(buffer,fp,shmid,shmid_1);
                //这里获得数据后，通过共享内存将数据传到顶层的父进程中去
            }
            // 关闭管道
            pclose(fp);
            shmdt(ptr);
            shmdt(ptr_1);
            exit(0);//退出
        }
    }


    else if(pid>0)//父
    {
        sleep(1);
        //初始化连接mysql服务器
        MYSQL *mysql;
        //内部有出错处理
        mysql = c_db_init();

        //这里做测试，用于给管理显示        
        //printf("test:%s\n",c_db_get(mysql,"SELECT * FROM login_info"));
        
        //服务器初始化相关变量
        int linkfd,val=1,epfd,ret,i,talkfd;
        struct sockaddr_in server_st;
        struct sockaddr_in client_st;//看这里要不要用了结构体数据来存放客户端的信息
        socklen_t client_st_len = sizeof(client_st);
        struct epoll_event tmp,events[EVENTSSIZE];

        //内部已经有出错处理
        tcp_init(&linkfd,&val,&server_st,&epfd,&tmp);
        
        //初始化线程池,目前有5个线程在等待客户端连接
        ret_pool = thread_pool_create(MIN_THR,MAX_THR,MAX_QUEUE);

        //记录服务器所处的状态，和连接数
        //uint8_t server_state=0;
        uint16_t connect_num = 0;

        //初始化共享内存,后续使用
        if((ptr = shmat(shmid,NULL,0))==(void *)-1)
        {
            sys_err("fail to shmat:");
            // perror("fail to shmat\n");
            // exit(1);
        }

        if((ptr_1 = shmat(shmid_1,NULL,0))==(void *)-1)
        {
            sys_err("fail to shmat_1:");
            // perror("fail to shmat\n");
            // exit(1);
        }

        //printf("aliyun_init_begin\n");
        void *device_client = init_rk_to_aliyun();

        while (1)
        {
            
            //下面这行的意思大概就是使用epoll监听epfd，有响应的事件就会存储到events里面,阻塞等待连接的客户端发来信息
            ret = epoll_wait(epfd,events,EVENTSSIZE,-1);//返回值就是触发io响应的事件数
            if(ret<0)
            {
                sys_err("epoll_wait:");
            }
            for(i=0;i<ret;i++)
            {
                //根据文件描述符来区分
                if(events[i].data.fd == linkfd)
                {
                    connect_num++; //记录连接客户端数量
                    //对linkfd进行io访问的就是创建新的连接
                    talkfd = accept(linkfd,(struct sockaddr *)&client_st,&client_st_len);
                    //对新建的连接增添信息
                    //printf("connetc from ip:%s port:%d\n",inet_ntoa(client_st.sin_addr),ntohl(client_st.sin_port));
                    tmp.data.fd = talkfd;
                    tmp.events = EPOLLIN;
                    if(epoll_ctl(epfd,EPOLL_CTL_ADD,talkfd,&tmp))
                    {
                        sys_err("epoll_ctl:");
                    }
                    if(send(talkfd,"connected",10,0)==-1)
                    {
                        sys_err("send:");
                    }
                    struct thread_info_st *thread_info_st_1 = (struct thread_info_st *)malloc(sizeof(struct thread_info_st));
                    //thread_info_st_1->ret = &ret;
                    //thread_info_st_1->epoll_event = events;
                    //thread_info_st_1->server_state = server_state;
                    //thread_info_st_1->pipefd = pipefd;//传指针
                    //thread_info_st_1->buffer = buffer;

                    thread_info_st_1->talkfd = talkfd;
                    thread_info_st_1->epfd = epfd;
                    thread_info_st_1->tmp = tmp;
                    thread_info_st_1->mysql = mysql;
                    thread_info_st_1->device_client = device_client;
                    thread_info_st_1->ptr = ptr;
                    thread_info_st_1->ptr_1 =ptr_1;
                    //这里添加到线程池里面,最后一个是想传的参数,注意这里传入的参数是所有线程公用的，如果要私自使用
                    thread_pool_addclient(ret_pool,fun_process_client,thread_info_st_1);
                }   
            }
        }
    }
    return 0;
}

int tcp_init(int *linkfd,int *val,struct sockaddr_in* server_st,int *epfd,struct epoll_event *tmp)
{
    *linkfd = socket(AF_INET,SOCK_STREAM,0); //IPV4 的tcp连接
    if(*linkfd==-1)
    {
        sys_err("fail to socket");
    }
    //防止服务器重启进入timewait无法绑定窗口，需设置重用地址
    if(setsockopt(*linkfd,IPPROTO_IP,SO_REUSEADDR,val,sizeof(*val))==-1)
    {
        sys_err("fail to setsockopt");
    }
    (*server_st).sin_family = AF_INET;//IP
    (*server_st).sin_port = htons(atoi(PORT));//先转换为整形，再转换为网络字节序
    (*server_st).sin_addr.s_addr = htonl(INADDR_ANY);//服务器地址随机分配一个 htonl(atoi(IP));//
    if(bind(*linkfd,(struct sockaddr*)server_st,sizeof((*server_st)))) //将linkfd和服务器结构体绑定
    {
        sys_err("bind()");
    }
    if(listen(*linkfd,128))//128只是建议值，系统会调整
    {
        sys_err("listen()");
    }
    *epfd = epoll_create1(EPOLL_CLOEXEC); //文件描述符在调用exec时关闭，防止文件描述符在程序中泄漏
    if(*epfd<0)
    {
        sys_err("fail to epoll_create1");
    }
    (*tmp).events = EPOLLIN;//监听的事件
    (*tmp).data.fd = *linkfd;//监听事件对应的文件描述符
    if(epoll_ctl(*epfd,EPOLL_CTL_ADD,*linkfd,tmp))//向epoll中添加一个关于linkfd的监听事件
    {
        sys_err("epoll_ctl()1");
    }
}

void cloud_info_trans_process(char *buffer,FILE *fp,int shmid,int shmid_1)
{

    if(fgets(buffer, BUFFER_SIZE, fp) != NULL)
    {
        //printf("rec:%s\n", buffer);
        if(strncmp(buffer,"received a message",18)==0)
        {
            printf("in\n");
            //处理部分
            char *search_str = "\"value\"";
            char *temp_char = buffer;
            char per_info[20] ;
            char trans_info[120];
            char *ptr=NULL;
            char *ptr_1=NULL;
            memset(trans_info,0,120);
            while ((temp_char=strstr(temp_char,search_str))!=NULL)
            {
                //printf("str:%s\n",temp_char);
                
                char *extract_str = extract_between(temp_char,':',',');
                if(extract_str==NULL)
                {
                    printf("extract_str NULL\n");
                }

                //printf("extract str:%s\n",extract_str);
                sprintf(per_info,"%s:",extract_str);
                strcat(trans_info,per_info);
                temp_char += strlen(search_str);
            }

            //printf("%s\n",trans_info);
            if((ptr = shmat(shmid,NULL,0))==(void *)-1)
            {
                sys_err("fail to shmat:");
                // perror("fail to shmat\n");
                // exit(1);
            }
            //printf("the data:%s\n",trans_info);
            if(strcmp(ptr,"")!=0)//不为空
            {
                memset(ptr,0,MEMSIZE);//不为空先清空
            }
            strcpy(ptr,trans_info);
            

            if((ptr_1 = shmat(shmid_1,NULL,0))==(void *)-1)
            {
                sys_err("fail to shmat_1:");
                // perror("fail to shmat\n");
                // exit(1);
            }
            if(strcmp(ptr_1,"")!=0)//不为空
            {
                memset(ptr_1,0,MEMSIZE);//不为空先清空
            }
            char update[13]="data_update";
            strcpy(ptr_1,update);
            // printf("ptr_1:%s\n",ptr_1);
            // printf("ptr_1_r:%s\n",update);
            // printf("test\n");
            //发送信号给主进程
            //kill(getppid(), SIGUSR1);
            //close(pipefd[0]); // 关闭读端
            //write(pipefd[1], "data_update", 12); // 向管道写数据,这里会触发服务器主进程的更新数据操作
            printf("data updata\n");
        }
        else
        {
            memset(buffer,0,BUFFER_SIZE);
            return;
        }
        
    }
}

void case2_set_data_process(char *info,void *device_client)
{
     //客户端设置逻辑
    //printf("client_set_info\n");
    //char *sonic_s,*buzzer_s,*led_s,*door_s,*fan_s;
    char *rec_s_array[5];
    //int8_t upload_state[5];
    //这么做是因为strtok会修改原字符串，将分隔符换为结束符来实现，所以每次只能用新的来进行strtok
    char *temp_info1 = (char *)malloc(strlen(info)+1);
    strcpy(temp_info1,info);
    strtok(temp_info1,":");
    for(int i=0;i<5;i++)
    {
        rec_s_array[i]=strtok(NULL,":");
    }

    //组织数据
    char *pub_topic = "/"PRODUCT_KEY"/"DEVICE_NAME"/user/rk3568_set_info";
    char temp_set_char[100];
    //printf("info:%s\n",rec_s_array[4]);
    sprintf(temp_set_char,"{\"s_s\": %d, \"b_s\": %d, \"l_s\": %d, \"d_s\": %d, \"m_s\": %d, \"TargetDevice\": \"mqtt_stm32\"}",check_state(rec_s_array[0]),check_state(rec_s_array[1]),check_state(rec_s_array[2]),check_state(rec_s_array[3]),check_state(rec_s_array[4]));
    aiot_msg_t *pub_message = aiot_msg_create_raw(pub_topic, (uint8_t *)temp_set_char, strlen(temp_set_char));

    /* 发送消?? */
    aiot_device_send_message(device_client, pub_message);

    /* 删除消息 */
    aiot_msg_delete(pub_message);
    //发送即可
    printf("send set to aliyun successfully\n");
    //释放内存
    free(temp_info1);
}

void case2_request_data_process(int talkfd,char *ptr)
{   
    char trans_info[100];
    if(strcmp(ptr,"")==0) //这里的ptr是接收子进程解析aliyun数据的共享内存区域
    {
        sprintf(trans_info,"empty_data");
        if(send(talkfd,trans_info,strlen(trans_info)+1,0)<0)
        {
            perror("fail to send sqlinfo\n");
            return;
            //break;//跳到switch外面，for的里面
        }
        printf("empty data\n");
    }
    else
    {
        sprintf(trans_info,"return_data:%s",ptr);//这里也许不会，因为是使用的拼接，没有把数据从缓冲区取出
        //strncpy(trans_info,ptr,strlen(ptr)+1);
        //printf("trans_info:%s\n",trans_info);
        if(send(talkfd,trans_info,strlen(trans_info)+1,0)<0)
        {
            perror("fail to send sqlinfo\n");
            return;
            //break;//跳到switch外面，for的里面
        }
        printf("trans_info successfully\n");
    }
   
}


void case1_goverment_delete_process(char *info,MYSQL *mysql,int talkfd)
{
    char *name;
    //这么做是因为strtok会修改原字符串，将分隔符换为结束符来实现，所以每次只能用新的来进行strtok
    char *temp_info1 = (char *)malloc(strlen(info)+1);
    strcpy(temp_info1,info);
    name = strtok(temp_info1,":");
    if(c_db_select(mysql,"SELECT * FROM login_info",name,NULL,NULL)==2)
    {
        printf("find\n");
        char delete_sql[50];
        sprintf(delete_sql,"delete from login_info where name = '%s'",name);
        c_db_delete(mysql,delete_sql);
        
        //更新数据库
        char *sendinfo = c_db_get(mysql,"SELECT * FROM login_info");
        char *new_sendinfo = (char *)malloc(strlen(sendinfo)+7);
        sprintf(new_sendinfo,"manage%s\n",sendinfo);
        
        //printf("%s%d\n",new_sendinfo,strlen(new_sendinfo));

        
        if(send(talkfd,new_sendinfo,strlen(new_sendinfo)+1,0)<0)
        {
            perror("fail to send sqlinfo\n");
        }
        free(new_sendinfo);
        free(sendinfo);
    }
    else
    {
        printf("not find\n");
    }
    free(temp_info1);
}



//这个函数会莫名其妙报段错误
void case1_goverment_change_process(char *info,MYSQL *mysql,int talkfd)
{
    printf("\n");
    char *name,*id,*account,*password;
    //这么做是因为strtok会修改原字符串，将分隔符换为结束符来实现，所以每次只能用新的来进行strtok
    if (info == NULL) {
        fprintf(stderr, "Error: info is NULL\n");
        return;
    }

    char *temp_info1 = (char *)malloc(strlen(info)+1);
    if (temp_info1 == NULL) {
        fprintf(stderr, "Error: memory allocation for temp_info1 failed\n");
        return;
    }

    strcpy(temp_info1,info);

    printf("\n");
    name = strtok(temp_info1,":");
    id = strtok(NULL,":");
    account = strtok(NULL,":");
    password = strtok(NULL,":");
    
    if (name == NULL || id == NULL || account == NULL || password == NULL) {
        fprintf(stderr, "Error: incomplete info string\n");
        free(temp_info1);
        return;
    }

    if(c_db_select(mysql,"SELECT * FROM login_info",name,NULL,NULL)==2)
    {
        char update_sql[300];
        sprintf(update_sql,"update login_info set id='%s', account='%s', password='%s' where name='%s'",id,account,password,name);
        //对数据库进行修改
        c_db_update(mysql,update_sql);

        //更新数据库
        char *sendinfo = c_db_get(mysql,"SELECT * FROM login_info");
        char *new_sendinfo = (char *)malloc(strlen(sendinfo)+7);
        sprintf(new_sendinfo,"manage%s\n",sendinfo);
        
        //printf("%s%d\n",new_sendinfo,strlen(new_sendinfo));
        if(send(talkfd,new_sendinfo,strlen(new_sendinfo)+1,0)<0)
        {
            perror("fail to send sqlinfo\n");
        }
        if(new_sendinfo!=NULL)
        {
            free(new_sendinfo);
        }
        if(sendinfo!=NULL)
        {
            free(sendinfo);
        }
        
        //printf("goverment:yes\n");
    }
    else
    {
        send(talkfd,"goverment:no",13,0);
        //printf("goverment:no\n");
    }
    //释放
    if(temp_info1!=NULL)
    {
        free(temp_info1);
    }
    
}


void case1_goverment_login_process(MYSQL *mysql,int talkfd)
{
    char *sendinfo = c_db_get(mysql,"SELECT * FROM login_info");
    char *new_sendinfo = (char *)malloc(strlen(sendinfo)+7);
    sprintf(new_sendinfo,"manage%s\n",sendinfo);
    //printf("%s%d\n",new_sendinfo,strlen(new_sendinfo));

    send(talkfd,new_sendinfo,strlen(new_sendinfo)+1,0);
    free(new_sendinfo);
    free(sendinfo);//sql函数里面涉及到对内存的申请
}


void case1_register_process(char *info,MYSQL *mysql,int talkfd)
{
    //同为局部变量
    char *name,*account,*password;
    //这么做是因为strtok会修改原字符串，将分隔符换为结束符来实现，所以每次只能用新的来进行strtok
    char *temp_info1 = (char *)malloc(strlen(info)+1);
    strcpy(temp_info1,info);
    name = strtok(temp_info1,":");
    account = strtok(NULL,":");
    password = strtok(NULL,":");
    char select_sql[200];
    //这里的id使用随机分配，且不重复
    //使用sql的语句来应对数据库登录相关的问题
    sprintf(select_sql,"insert login_info(name,id,account,password) values('%s',FLOOR(RAND() * (999999 - 100000 + 1)) + 100000,'%s','%s')",name,account,password);//获得名字
    
    if(c_db_insert(mysql,select_sql)==0)
    {
        //printf("registe success\n");
        send(talkfd,"register:yes",13,0);
    }
    else
    {
        //printf("registe failed\n");
        send(talkfd,"register:no",12,0);
    }
    //释放
    free(temp_info1);
}


void case1_login_process(char *info,MYSQL *mysql,int talkfd)
{
    //跟strtok相关的指针都不用释放，其是在原字符串基础上得到的，只要原始字符串不是malloc的就不用释放
    char *name,*account,*password;
    //这么做是因为strtok会修改原字符串，将分隔符换为结束符来实现，所以每次只能用新的来进行strtok
    char *temp_info1 = (char *)malloc(strlen(info)+1);
    strcpy(temp_info1,info);
    name = strtok(temp_info1,":");
    account = strtok(NULL,":");
    password = strtok(NULL,":");
    //获取数据库的信息
    printf("%s:%s:%s\n",name,account,password);
    if(c_db_select(mysql,"SELECT * FROM login_info",name,account,password)==1)
    {
        //printf("login success\n");
        send(talkfd,"login:yes",10,0);
    }
    else
    {
        //printf("login failed\n");
        send(talkfd,"login:no",9,0);
    }
    //用完释放，防止泄露
    free(temp_info1);
}




char *get_info_tag(char *info)
{
    char *token;
    char *lasttoken;
    char *last_trans_token = (char *)malloc(strlen(info)+1);
    char *temp_info = (char *)malloc(strlen(info)+1);
    strcpy(temp_info,info);
    
    //这里注意strtok是在原字符串上进行操作，所有如果要释放函数中申请的内存，还要将其传到外面，就需要重新开辟一片空间，来传出去。
    token = strtok(temp_info,":");
    while(token!=NULL)
    {
        lasttoken = token;
        token = strtok(NULL,":");
        //printf("token:%s\n",token);                                                        
    }
    //传出新内存
    strcpy(last_trans_token,lasttoken);
    free(temp_info);//释放原内存
    return last_trans_token;
}

//tips:shift+tab使选中的多行向前缩进
int get_server_state(char *info)
{
    //这里的old_state会一直保存系统所处的状态，如果没改变就会将原状态给返回
    static int8_t old_state=0;
    char *temp_info_0 = (char *)malloc(strlen(info)+1);//这两行申请并且使用，不影响原接收字符串
    strcpy(temp_info_0,info);
    if(strstr(temp_info_0,"\n")!=NULL)
    {
        temp_info_0 = strtok(temp_info_0,"\n");
    }
    
    if(strncmp(temp_info_0,"state:",6)==0)
    {
        char *state;
        strtok(temp_info_0,":");
        state =  strtok(NULL,":");
        int8_t i=atoi(state);
        if(i==0 || i==1 ||i==2)
        {
            //client_state = i;
            if(old_state!=i)
            {
                printf("server change to state:%d\n",i);
                old_state = i;
            }
            //printf("in state %d\n",i);
        }
        else
        {
            printf("wrong server state change \n");
        }
    }
    free(temp_info_0);
    return old_state;

}


void *init_rk_to_aliyun()
{
    //mqtt连接阿里云的初始化工作
    int32_t     res = STATE_SUCCESS;
    /* 配置SDK的底层依赖 */
    aiot_sysdep_init(aiot_sysdep_get_portfile());

    /* 创建设备 */
    void *device_client = aiot_device_create(PRODUCT_KEY, DEVICE_NAME);
    if (device_client == NULL) {
        printf("device_client failed\n");
        exit(1);
    }

    /* 设置设备密钥 */
    aiot_device_set_device_secret(device_client, DEVICE_SECRET);
    aiot_linkconfig_t* config = aiot_linkconfig_init(MQTT_PROTOCOL);
    /* 设置服务器的host、port */
    aiot_linkconfig_host(config, HOST, PORT1);

    /* 设置设备连接参数 */
    aiot_device_set_linkconfig(device_client, config);
    /* 设备建连 */
    res = aiot_device_connect(device_client);
    if (res < STATE_SUCCESS) {
        /* 尝试建立连接失败, 销毁MQTT实例, 回收资源 */
        aiot_linkconfig_deinit(&config);
        aiot_device_delete(&device_client);
        printf("aiot_device_connect failed: -0x%04X\n\r\n", -res);
        //return -1;
        exit(1);
    }
    printf("aliyun_init_exit\n");
    return device_client;
}


int check_state(char *temp)
{
    if(strcmp(temp,"On")==0 || strcmp(temp,"Slow")==0)
    {
        return 1;
    }
    else if(strcmp(temp,"Off")==0)
    {
        return 0;
    }
    else if(strcmp(temp,"empty")==0)
    {
        return -1;
    }
    else if(strcmp(temp,"Fast")==0)
    {
        return 2;
    }
}

//fun_process_client程序,处理来自客户端的请求，先设计客户端的需求
//gcc rk3568_server.c connect_mysql.c -o rk3568_server -L/user/lib/mysql -lmysqlclient
