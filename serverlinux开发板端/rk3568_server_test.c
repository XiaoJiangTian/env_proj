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

#include "threadpool.h"
#include "connect_mysql.h"


#include "../device-core/aiot_device_api.h"
#include "demo_config.h"

#define IP "192.168.184.4"
#define PORT "9999"
#define EVENTSSIZE 1024//epoll能承受的连接

#define BUFFER_SIZE 128
#define MEMSIZE 1024

#define MIN_THR 5
#define MAX_THR 1024
#define MAX_QUEUE 1024

int check_state(char *temp);
void *init_rk_to_aliyun();

static void sys_err(char *s)
{
    perror(s);
    exit(1);
}



//服务器针对每个客户端的处理程序
void fun_process_client()
{
    
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

int main(int argc, char const *argv[])
{


    struct pool *ret;
    pid_t pid,pid_1;

    //共享内存
    int shmid;
    char *ptr;//共享内存区域
    shmid = shmget(IPC_PRIVATE,MEMSIZE,0664);//获取shmid
    if(shmid<0)
    {
        perror("fail to shmget\n");
        exit(1);
    }

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
            // FILE *fp;
            // char buffer[BUFFER_SIZE];
            // char command[BUFFER_SIZE];

            // // 构建要执行的命令
            // snprintf(command, sizeof(command), "python3 test.py");

            // // 执行命令并打开管道
            // fp = popen(command, "r");
            // if (fp == NULL) {
            //     perror("popen");
            //     return 1;
            // }

            // // 读取命令的输出
            // while (1) {
            //     if(fgets(buffer, sizeof(buffer), fp) != NULL)
            //     {
            //         printf("from son process:Output from Python script: %s", buffer);
            //     }

            //     //这里获得数据后，通过共享内存将数据传到顶层的父进程中去
            // }

            // // 关闭管道
            // pclose(fp);

            while(1)
            {
                sleep(1);
                // char *str="received a message \"{\"deviceType\":\"CustomCategory\",\"iotId\":\"oW44zFBKdNwyTiCv5p8Gk1m420\",\"requestId\":\"null\",\"checkFailedData\":{},\"productKey\":\"k1m42D6OwDF\",\"gmtCreate\":1722348908726,\"deviceName\":\"mqtt_stm32\", \
                // \"items\":{\"rain\":{\"value\":0,\"time\":1722348908722},\"led_state\":{\"value\":1,\"time\":1722348908722},\"dht11_temper\":{\"value\":29.3,\"time\":1722348908722},\"door_state\":{\"value\":0,\"time\":1722348908722}, \
                // \"motor_state\":{\"value\":0,\"time\":1722348908722},\"dht11_huminity\":{\"value\":51.0,\"time\":1722348908722}}}\"";

                char *str = "received a message \"{\"deviceType\":\"CustomCategory\",\"iotId\":\"oW44zFBKdNwyTiCv5p8Gk1m420\",\"requestId\":\"null\",\"checkFailedData\":{},\"productKey\":\"k1m42D6OwDF\",\"gmtCreate\":1723951226693,\"deviceName\":\"mqtt_stm32\",\"items\":{\"m_s\":{\"value\":0,\"time\":1723951226683},\"l_s\":{\"value\":1,\"time\":1723951226683},\"rain\":{\"value\":0,\"time\":1723951226683},\"dht11_t\":{\"value\":30.2,\"time\":1723951226683},\"s_s\":{\"value\":0,\"tOutput from Python script: time\":1723951226683},\"b_s\":{\"value\":0,\"time\":1723951226683},\"s_d\":{\"value\":14.43,\"time\":1723951226683},\"d_s\":{\"value\":0,\"time\":1723951226683},\"dht11_h\":{\"value\":42.0,\"time\":1723951226683}}}\"";
                char *search_str = "\"value\"";
                
                char *temp_char = str;
                char per_info[20] ;
                char trans_info[120];
                memset(trans_info,0,120);
                while ((temp_char=strstr(temp_char,search_str))!=NULL)
                {
                    //printf("str:%s\n",temp_char);
                    
                    char *extract_str = extract_between(temp_char,':',',');
                    //printf("extract str:%s\n",extract_str);
                    sprintf(per_info,"%s:",extract_str);
                    strcat(trans_info,per_info);
                    temp_char += strlen(search_str);
                }

                //printf("%s\n",trans_info);
                if((ptr = shmat(shmid,NULL,0))==(void *)-1)
                {
                    perror("fail to shmat\n");
                    exit(1);
                }
                strcpy(ptr,trans_info);
                //测试
                while (1)
                {
                    
                }
                
                shmdt(ptr);
                //break;
            }
            
            
            
            exit(0);//这里回推出


        }
    }

    else if(pid>0)//父
    {
        sleep(1);
        //初始化连接mysql服务器
        MYSQL *mysql;
        mysql = c_db_init();

        //这里做测试，用于给管理显示        
        //printf("test:%s\n",c_db_get(mysql,"SELECT * FROM login_info"));
        
        //服务器进程
        int linkfd,val=1,epfd,ret,i,talkfd;
        struct sockaddr_in server_st;
        struct sockaddr_in client_st;//看这里要不要用了结构体数据来存放客户端的信息
        socklen_t client_st_len = sizeof(client_st);

        struct epoll_event tmp,events[EVENTSSIZE];
        linkfd = socket(AF_INET,SOCK_STREAM,0); //IPV4 的tcp连接
        if(linkfd==-1)
        {
            sys_err("fail to socket");
        }
        //防止服务器重启进入timewait无法绑定窗口，需设置重用地址
        if(setsockopt(linkfd,IPPROTO_IP,SO_REUSEADDR,&val,sizeof(val))==-1)
        {
            sys_err("fail to setsockopt");
        }
        server_st.sin_family = AF_INET;//IP
        server_st.sin_port = htons(atoi(PORT));//先转换为整形，再转换为网络字节序
        server_st.sin_addr.s_addr = htonl(INADDR_ANY);//服务器地址随机分配一个 htonl(atoi(IP));//
        if(bind(linkfd,(struct sockaddr*)&server_st,sizeof(server_st))) //将linkfd和服务器结构体绑定
        {
            sys_err("bind()");
        }
        if(listen(linkfd,128))//128只是建议值，系统会调整
        {
            sys_err("listen()");
        }
        epfd = epoll_create1(EPOLL_CLOEXEC); //文件描述符在调用exec时关闭，防止文件描述符在程序中泄漏
        if(epfd<0)
        {
            sys_err("fail to epoll_create1");
        }
        tmp.events = EPOLLIN;//监听的事件
        tmp.data.fd = linkfd;//监听事件对应的文件描述符
        if(epoll_ctl(epfd,EPOLL_CTL_ADD,linkfd,&tmp))//向epoll中添加一个关于linkfd的监听事件
        {
            sys_err("epoll_ctl()1");
        }

        //初始化线程池,目前有5个线程在等待客户端连接
        //ret = thread_pool_create(MIN_THR,MAX_THR,MAX_QUEUE);
        
        
        uint8_t client_state=0;
        uint16_t connect_num = 0;
        //int row=0;

        //初始化共享内存
        if((ptr = shmat(shmid,NULL,0))==(void *)-1)
        {
            perror("fail to shmat\n");
            exit(1);
        }

        // //mqtt连接阿里云的初始化工作
        // int32_t     res = STATE_SUCCESS;
        // /* 配置SDK的底层依赖 */
        // aiot_sysdep_init(aiot_sysdep_get_portfile());

        // /* 创建设备 */
        // void *device_client = aiot_device_create(PRODUCT_KEY, DEVICE_NAME);
        // if (device_client == NULL) {
        //     printf("device_client failed\n");
        //     return -1;
        // }

        // /* 设置设备密钥 */
        // aiot_device_set_device_secret(device_client, DEVICE_SECRET);
        // aiot_linkconfig_t* config = aiot_linkconfig_init(MQTT_PROTOCOL);
        // /* 设置服务器的host、port */
        // aiot_linkconfig_host(config, HOST, PORT1);

        // /* 设置设备连接参数 */
        // aiot_device_set_linkconfig(device_client, config);
        // /* 设备建连 */
        // res = aiot_device_connect(device_client);
        // if (res < STATE_SUCCESS) {
        //     /* 尝试建立连接失败, 销毁MQTT实例, 回收资源 */
        //     aiot_linkconfig_deinit(&config);
        //     aiot_device_delete(&device_client);
        //     printf("aiot_device_connect failed: -0x%04X\n\r\n", -res);
        //     return -1;
        // }

        void *device_client = init_rk_to_aliyun();

        while (1)
        {
            //printf("run\n");
            

            //测试使用
            //printf("from son process:%s\n",ptr);
            // shmdt(ptr);//接触映射
            // //去除shmid
            // if(shmctl(shmid,IPC_RMID,0)<0)
            // {
            //     perror("fail to shmctl\n");
            //     exit(1);
            // }
            
            //下面这行的意思大概就是使用epoll监听epfd，有响应的事件就会存储到events里面
            ret = epoll_wait(epfd,events,EVENTSSIZE,-1);//返回值就是触发io响应的事件数
            if(ret<0)
            {
                sys_err("epoll_wait()");
            }
            //printf("ret:%d\n",ret);
            for(i=0;i<ret;i++)
            {
                //根据文件描述符来区分
                if(events[i].data.fd == linkfd)
                {
                    connect_num++;
                    //对linkfd进行io访问的就是创建新的连接
                    talkfd = accept(linkfd,(struct sockaddr *)&client_st,&client_st_len);
                    //对新建的连接增添信息
                    //printf("connetc from ip:%s port:%d\n",inet_ntoa(client_st.sin_addr),ntohl(client_st.sin_port));
                    tmp.data.fd = talkfd;
                    tmp.events = EPOLLIN;
                    if(epoll_ctl(epfd,EPOLL_CTL_ADD,talkfd,&tmp))
                    {
                        sys_err("epoll_ctl()2");
                    }
                    // //返回已连接客户端数量
                    // char connect_text[20];
                    // sprintf(connect_text,"connect_num:%d",connect_num);
                    send(talkfd,"connected",10,0);
                    //这里添加到线程池里面
                    //thread_pool_addclient(ret,fun_process_client,NULL);
                }
                else//已经建立连接，进行通信的fd，这里应该交给线程池去做
                {
                    //这里可以做一些提示性的语句，比如某某客户端进行了请求
                    char info[1024];
                    memset(info,0,1024);
                    //每次使用接收缓冲区时，都需将其清空，防止之前数据的影响
                    //printf("data request from the client socketfd:%d\n",events[i].data.fd);
                    if(recv(events[i].data.fd,info,1024,0)==0)
                    {
                        close(events[i].data.fd);
                        epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,&tmp);
                        connect_num--;
                        send(talkfd,"connect_num:%d",connect_num,0);
                        printf("client exit\n");
                        continue;
                    }

                    printf("info:%s\n",info);
                    //send(events[i].data.fd,"i receive your message",24,0);

                    char *temp_info_0 = (char *)malloc(sizeof(info));
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
                        uint16_t i=atoi(state);
                        if(i==0 || i==1 ||i==2)
                        {
                            client_state = i;
                            printf("in state %d\n",client_state);
                        }
                        else
                        {
                            printf("wrong state\n");
                        }
                    }
                    free(temp_info_0);


                    switch (client_state)
                    {

                        //网络连接界面
                        case 0:
                        {
                            printf("case0\n");
                        }
                        break;
                        //登录界面处理
                        case 1:
                        {
                            printf("case1\n");
                            char *token;
                            char *lasttoken=NULL;
                            char *temp_info = (char *)malloc(sizeof(info));
                            strcpy(temp_info,info);
                            
                            token = strtok(temp_info,":");
                            while(token!=NULL)
                            {
                                lasttoken = token;
                                token = strtok(NULL,":");
                                //printf("token:%s\n",token);                                                        
                                                        //这里为什么可以这么操作，因为变的是指针而不是内容，
                                                         //如果两个指向都不变，修改其中任何一个的内容，其他都会变，但是改指针指向，另一个不会跟着变
                                                         //还是指向原来的内存区域
                            }
                            
                            //printf("the last str is :%s\n",lasttoken); 测试语句
                            //分情况处理
                            //处理登录情况login
                            //只会走其中一个分支，所以每个都需要重新定义变量
                            if(strcmp(lasttoken,"login")==0)
                            {
                                printf("login\n");
                                //跟strtok相关的指针都不用释放，其是在原字符串基础上得到的，只要原始字符串不是malloc的就不用释放
                                char *name,*account,*password;
                                //这么做是因为strtok会修改原字符串，将分隔符换为结束符来实现，所以每次只能用新的来进行strtok
                                char *temp_info1 = (char *)malloc(sizeof(info));
                                strcpy(temp_info1,info);
                                name = strtok(temp_info1,":");
                                account = strtok(NULL,":");
                                password = strtok(NULL,":");
                                
                                //获取数据库的信息
                                printf("%s:%s:%s\n",name,account,password);
                                if(c_db_select(mysql,"SELECT * FROM login_info",name,account,password)==1)
                                {
                                    //printf("login success\n");
                                    send(events[i].data.fd,"login:yes",10,0);

                                }
                                else
                                {
                                    //printf("login failed\n");
                                    send(events[i].data.fd,"login:no",9,0);
                                }

                                //释放
                                free(temp_info1);
                            }
                            //注册情况
                            else if(strcmp(lasttoken,"register")==0)
                            {
                                printf("register\n");
                                //同为局部变量
                                char *name,*account,*password;
                                //这么做是因为strtok会修改原字符串，将分隔符换为结束符来实现，所以每次只能用新的来进行strtok
                                char *temp_info1 = (char *)malloc(sizeof(info));
                                strcpy(temp_info1,info);
                                name = strtok(temp_info1,":");
                                account = strtok(NULL,":");
                                password = strtok(NULL,":");
                                char select_sql[200];
                                //这里的id使用随机分配，且不重复
                                sprintf(select_sql,"insert login_info(name,id,account,password) values('%s',FLOOR(RAND() * (999999 - 100000 + 1)) + 100000,'%s','%s')",name,account,password);//获得名字
                                
                                if(c_db_insert(mysql,select_sql)==0)
                                {
                                    //printf("registe success\n");
                                    send(events[i].data.fd,"registe:yes",12,0);
                                }
                                else
                                {
                                    //printf("registe failed\n");
                                    send(events[i].data.fd,"registe:no",11,0);
                                }
                                //释放
                                free(temp_info1);
                            }
                            //管理情况
                            else if(strcmp(lasttoken,"goverment")==0)
                            {
                               
                                printf("goverment_login\n");
                                char *sendinfo = c_db_get(mysql,"SELECT * FROM login_info");
                                char *new_sendinfo = (char *)malloc(strlen(sendinfo)+7);
                                sprintf(new_sendinfo,"manage%s\n",sendinfo);
                                //printf("%s%d\n",new_sendinfo,strlen(new_sendinfo));
                                send(events[i].data.fd,new_sendinfo,strlen(new_sendinfo)+1,0);
                                free(new_sendinfo);
                                free(sendinfo);
                            }

                            //管理内容进行修改
                            else if(strcmp(lasttoken,"goverment_change")==0)
                            {
                               
                                printf("goverment_change\n");
                                char *name,*id,*account,*password;
                                //这么做是因为strtok会修改原字符串，将分隔符换为结束符来实现，所以每次只能用新的来进行strtok
                                char *temp_info1 = (char *)malloc(sizeof(info));
                                strcpy(temp_info1,info);

                                name = strtok(temp_info1,":");
                                id = strtok(NULL,":");
                                account = strtok(NULL,":");
                                password = strtok(NULL,":");
                                
                                if(c_db_select(mysql,"SELECT * FROM login_info",name,NULL,NULL)==2)
                                {
                                    char update_sql[200];
                                    sprintf(update_sql,"update login_info set id='%s', account='%s', password='%s' where name='%s'",id,account,password,name);
                                    //对数据库进行修改
                                    c_db_update(mysql,update_sql);
                                    


                                    //更新数据库
                                    char *sendinfo = c_db_get(mysql,"SELECT * FROM login_info");
                                    char *new_sendinfo = (char *)malloc(strlen(sendinfo)+7);
                                    sprintf(new_sendinfo,"manage%s\n",sendinfo);
                                    
                                    //printf("%s%d\n",new_sendinfo,strlen(new_sendinfo));
                                    if(send(events[i].data.fd,new_sendinfo,strlen(new_sendinfo)+1,0)<0)
                                    {
                                        perror("fail to send sqlinfo\n");
                                    }
                                    free(new_sendinfo);
                                    free(sendinfo);
                                    //printf("goverment:yes\n");
                                }
                                else
                                {
                                    send(events[i].data.fd,"goverment:no",13,0);
                                    //printf("goverment:no\n");
                                }

                                //释放
                                free(temp_info1);
                                //return 2才是对的
                            }

                            else if(strcmp(lasttoken,"goverment_delete")==0)
                            {
                                printf("goverment_delete\n");
                                char *name;
                                //这么做是因为strtok会修改原字符串，将分隔符换为结束符来实现，所以每次只能用新的来进行strtok
                                char *temp_info1 = (char *)malloc(sizeof(info));
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

                                    
                                    if(send(events[i].data.fd,new_sendinfo,strlen(new_sendinfo)+1,0)<0)
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
                            free(temp_info);//防止内存泄漏

                            break;
                        }
                        break;

                        //登录成功后的处理逻辑,处理请求数据
                        case 2:
                        {
                            printf("case2\n");
                            //获取数据
                            if(strcmp(info,"request_data")==0)
                            {
                                printf("client_get_info\n");
                                char trans_info[100];
                                sprintf(trans_info,"return_data:%s",ptr);
                                //strncpy(trans_info,ptr,strlen(ptr)+1);
                                printf("trans_info:%s\n",trans_info);
                                if(send(events[i].data.fd,trans_info,strlen(trans_info)+1,0)<0)
                                {
                                    perror("fail to send sqlinfo\n");
                                    break;//跳到switch外面，for的里面
                                }
                                printf("trans_info successfully\n");
                            }
                            else if(strncmp(info,"set_data:",9)==0)
                            {
                                //客户端设置逻辑
                                printf("client_set_info\n");
                                //char *sonic_s,*buzzer_s,*led_s,*door_s,*fan_s;
                                char *rec_s_array[5];
                                int8_t upload_state[5];
                                //这么做是因为strtok会修改原字符串，将分隔符换为结束符来实现，所以每次只能用新的来进行strtok
                                char *temp_info1 = (char *)malloc(sizeof(info));
                                strcpy(temp_info1,info);
                                strtok(temp_info1,":");
                                for(int i=0;i<5;i++)
                                {
                                    rec_s_array[i]=strtok(NULL,":");
                                }
                                // sonic_s = strtok(NULL,":");
                                // buzzer_s = strtok(NULL,":");
                                // led_s = strtok(NULL,":");
                                // door_s = strtok(NULL,":");
                                // fan_s = strtok(NULL,":");

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

                            }
                        }
                        break;
                    
                    
                    }
                }
            }


        }
        

    }
    return 0;
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
        return -1;
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
