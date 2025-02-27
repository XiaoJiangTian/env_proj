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
#include <signal.h>


#include "threadpool.h"
#include "connect_mysql.h"


#include "../device-core/aiot_device_api.h"
#include "demo_config.h"

#define IP "192.168.184.4"
#define PORT "9999"
#define EVENTSSIZE 1024//epoll�ܳ��ܵ�����

#define BUFFER_SIZE 2000
#define MEMSIZE 1024

#define MIN_THR 5
#define MAX_THR 1024
#define MAX_QUEUE 1024

int check_state(char *temp);

void *init_rk_to_aliyun();
char *get_info_tag(char *info);
int get_server_state(char *info);

void case1_login_process(char *info,MYSQL *mysql,struct epoll_event *events,int i);
void case1_register_process(char *info,MYSQL *mysql,struct epoll_event *events,int i);
void case1_goverment_login_process(MYSQL *mysql,struct epoll_event *events,int i);
void case1_goverment_change_process(char *info,MYSQL *mysql,struct epoll_event *events,int i);
void case1_goverment_delete_process(char *info,MYSQL *mysql,struct epoll_event *events,int i);

void case2_request_data_process(struct epoll_event *events,int i,char *ptr);
void case2_set_data_process(char *info,void *device_client);

void cloud_info_trans_process(char *buffer,FILE *fp,int shmid,int *pipefd);

int tcp_init(int *linkfd,int *val,struct sockaddr_in* server_st,int *epfd,struct epoll_event *tmp);

static void sys_err(char *s)
{
    perror(s);
    exit(1);
}



//���������ÿ���ͻ��˵Ĵ������
//�ŵ��̳߳���Ļ�����Ҫɸѡһ�£���ǰ���talkfd��¼һ�£�����ÿ���߳�ֻ�����Ӧ�Ի���fd���з�Ӧ
void fun_process_client()
{
    
}

char * extract_between(const char *str,char start_char,char end_char)
{
    char *start = strchr(str,start_char); //���ַ��������ַ��ĺ���
    if(start==NULL)
    {
        return NULL;
    }
    //printf("%s\n",start);
    //�о�������ǰ�ҵ����ַ�
    start++;

    char *end = strchr(start,end_char);
    if(end==NULL)
    {
        return NULL;
    }

    size_t len = end - start;
    //�������ڴ����ڴ洢
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
        // ��ֹ�ӽ���
        kill(pid, SIGKILL);
        printf("Child process 1 terminated\n");
    }
    if(pid_1>0)
    {
        kill(pid_1, SIGKILL);
        printf("Child process 2 terminated\n");
    }
    exit(0); // ��ֹ������
}


int main(int argc, char const *argv[])
{


    struct pool *ret;

    //�����ڴ�
    int shmid;
    char *ptr=NULL;//�����ڴ�����
    shmid = shmget(IPC_PRIVATE,MEMSIZE,0664);//��ȡshmid
    if(shmid<0)
    {
        perror("fail to shmget\n");
        exit(1);
    }

    //����һ���ܵ������ӽ���֪ͨ���������ݸ���
    int pipefd[2];
    char buffer[15];
    if(pipe(pipefd)==-1)
    {
        perror("pipe");
        return -1;
    }
    fcntl(pipefd[0], F_SETFL, O_NONBLOCK);
    signal(SIGINT, handle_sigint);

    //ע��һ���źŴ������������������յ���Ϣ���Զ����µ��ͻ���
    //signal(SIGUSR1, handle_custom_signal);

    pid = fork();//���������
    if(pid == -1)
    {
        sys_err("fail to fork_1");
    }
    else if(pid==0)//��
    {
        pid_1 = fork();//�ڶ���fork
        if(pid_1==-1)
        {
            sys_err("fail to fork_2");
        }
        else if(pid_1 == 0) //�ڶ��ε���
        {
            //���������Ӧ�ò�
        }
        else if(pid_1>0) //�ڶ��εĸ�
        {
            //aliyun���ܺʹ��ͷ���
            FILE *fp;
            char buffer[BUFFER_SIZE];
            char command[BUFFER_SIZE];

            // ����Ҫִ�е�����
            snprintf(command, sizeof(command), "python3 test.py");

            // ִ������򿪹ܵ�
            fp = popen(command, "r");
            if (fp == NULL) {
                perror("popen");
                return 1;
            }
            // ��ȡ��������
            while (1) {
                cloud_info_trans_process(buffer,fp,shmid,pipefd);
                //���������ݺ�ͨ�������ڴ潫���ݴ�������ĸ�������ȥ
            }

            // �رչܵ�
            pclose(fp);
            shmdt(ptr);
            exit(0);//�˳�
        }
    }


    else if(pid>0)//��
    {
        sleep(1);
        //��ʼ������mysql������
        MYSQL *mysql;
        mysql = c_db_init();

        //���������ԣ����ڸ�������ʾ        
        //printf("test:%s\n",c_db_get(mysql,"SELECT * FROM login_info"));
        
        //��������ʼ����ر���
        int linkfd,val=1,epfd,ret,i,talkfd;
        struct sockaddr_in server_st;
        struct sockaddr_in client_st;//������Ҫ��Ҫ���˽ṹ����������ſͻ��˵���Ϣ
        socklen_t client_st_len = sizeof(client_st);
        struct epoll_event tmp,events[EVENTSSIZE];

        tcp_init(&linkfd,&val,&server_st,&epfd,&tmp);
        
        //��¼������������״̬����������
        uint8_t server_state=0;
        uint16_t connect_num = 0;

        //��ʼ�������ڴ�,����ʹ��
        if((ptr = shmat(shmid,NULL,0))==(void *)-1)
        {
            perror("fail to shmat\n");
            exit(1);
        }

        
        void *device_client = init_rk_to_aliyun();

        while (1)
        {
            
            //�������е���˼��ž���ʹ��epoll����epfd������Ӧ���¼��ͻ�洢��events����,�����ȴ����ӵĿͻ��˷�����Ϣ
            ret = epoll_wait(epfd,events,EVENTSSIZE,-1);//����ֵ���Ǵ���io��Ӧ���¼���
            if(ret<0)
            {
                sys_err("epoll_wait()");
            }
            //printf("ret:%d\n",ret);
            for(i=0;i<ret;i++)
            {
                //�����ļ�������������
                if(events[i].data.fd == linkfd)
                {
                    connect_num++; //��¼���ӿͻ�������
                    //��linkfd����io���ʵľ��Ǵ����µ�����
                    talkfd = accept(linkfd,(struct sockaddr *)&client_st,&client_st_len);
                    //���½�������������Ϣ
                    //printf("connetc from ip:%s port:%d\n",inet_ntoa(client_st.sin_addr),ntohl(client_st.sin_port));
                    tmp.data.fd = talkfd;
                    tmp.events = EPOLLIN;
                    if(epoll_ctl(epfd,EPOLL_CTL_ADD,talkfd,&tmp))
                    {
                        sys_err("epoll_ctl()2");
                    }
                    send(talkfd,"connected",10,0);
                    //������ӵ��̳߳�����
                    //thread_pool_addclient(ret,fun_process_client,NULL);
                }
                else//�Ѿ��������ӣ�����ͨ�ŵ�fd������Ӧ�ý����̳߳�ȥ��
                {
                    //���������һЩ��ʾ�Ե���䣬����ĳĳ�ͻ��˽���������
                    char info[1024];//������Ϣ�Ļ�����
                    memset(info,0,1024);//ÿ��ʹ�ý��ջ�����ʱ�����轫����գ���ֹ֮ǰ���ݵ�Ӱ��
                    
                    if(recv(events[i].data.fd,info,1024,0)==0) //�������ӿͻ����˳��ĳ���
                    {
                        close(events[i].data.fd);
                        epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,&tmp);
                        connect_num--;
                        send(talkfd,"connect_num:%d",connect_num,0);
                        printf("client exit\n");
                        continue;
                    }

                    printf("info:%s\n",info); //���ڵ���ʹ�ã���ӡ�����ӵĸ��ͻ��˷�������Ϣ
                    
                    //�������״̬�л������͸���״ֵ̬
                    int8_t temp_state = get_server_state(info);
                    printf("temp_state:%d\n",temp_state);
                    if(temp_state!=-1)
                    {
                        server_state = temp_state;
                    }
                    

                    switch (server_state)
                    {

                        //�������ӽ���
                        case 0:
                        {
                            printf("server in case 0\n");
                        }
                        break;
                        //��¼���洦��
                        case 1:
                        {
                            printf("server in case 1\n");
                            
                            //��ȡβ��tag
                            char *lasttoken = NULL;
                            lasttoken = get_info_tag(info);
                            
                            //���������
                            //�����¼���login
                            //ֻ��������һ����֧������ÿ������Ҫ���¶������
                            if(strcmp(lasttoken,"login")==0)
                            {
                                printf("case 1:login\n");
                                //�����¼��ص�sql����
                                case1_login_process(info,mysql,events,i);
                            }
                            //ע�����
                            else if(strcmp(lasttoken,"register")==0)
                            {
                                printf("case 1:register\n");
                                case1_register_process(info,mysql,events,i);
                               
                            }
                            //�������
                            else if(strcmp(lasttoken,"goverment")==0)
                            {
                                printf("case 1:goverment_login\n");
                                case1_goverment_login_process(mysql,events,i);
                            }

                            //�������ݽ����޸�
                            else if(strcmp(lasttoken,"goverment_change")==0)
                            {
                               
                                printf("case 1:goverment_change\n");
                                case1_goverment_change_process(info,mysql,events,i);

                            }
                            else if(strcmp(lasttoken,"goverment_delete")==0)
                            {
                                printf("case 1:goverment_delete\n");
                                case1_goverment_delete_process(info,mysql,events,i);
                            }
                            free(lasttoken);//��ֹ�ڴ�й©
                            break;
                        }
                        break;

                        //��¼�ɹ���Ĵ����߼�,������������
                        case 2:
                        {
                            printf("server in case2\n");

                            //��ȡ�ܵ����ݿ��Ƿ����
                            //close(pipefd[1]); // �ر�д��
                            read(pipefd[0], buffer, sizeof(buffer)); // ��ȡ�ܵ��е�����

                            printf("pipe received: %s\n", buffer);
                            //��ȡ����,�����ڽ��յ����ݵ�ʱ���Զ����и��²��Ҵ���
                            if(strcmp(info,"request_data")==0 || strcmp(buffer,"data_update")==0)
                            {
                                memset(buffer,0,15);
                                printf("case 2:request_data\n");
                                case2_request_data_process(events,i,ptr);

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
        }
    }
    return 0;
}

int tcp_init(int *linkfd,int *val,struct sockaddr_in* server_st,int *epfd,struct epoll_event *tmp)
{
    *linkfd = socket(AF_INET,SOCK_STREAM,0); //IPV4 ��tcp����
    if(*linkfd==-1)
    {
        sys_err("fail to socket");
    }
    //��ֹ��������������timewait�޷��󶨴��ڣ����������õ�ַ
    if(setsockopt(*linkfd,IPPROTO_IP,SO_REUSEADDR,val,sizeof(*val))==-1)
    {
        sys_err("fail to setsockopt");
    }
    (*server_st).sin_family = AF_INET;//IP
    (*server_st).sin_port = htons(atoi(PORT));//��ת��Ϊ���Σ���ת��Ϊ�����ֽ���
    (*server_st).sin_addr.s_addr = htonl(INADDR_ANY);//��������ַ�������һ�� htonl(atoi(IP));//
    if(bind(*linkfd,(struct sockaddr*)server_st,sizeof((*server_st)))) //��linkfd�ͷ������ṹ���
    {
        sys_err("bind()");
    }
    if(listen(*linkfd,128))//128ֻ�ǽ���ֵ��ϵͳ�����
    {
        sys_err("listen()");
    }
    *epfd = epoll_create1(EPOLL_CLOEXEC); //�ļ��������ڵ���execʱ�رգ���ֹ�ļ��������ڳ�����й©
    if(*epfd<0)
    {
        sys_err("fail to epoll_create1");
    }
    (*tmp).events = EPOLLIN;//�������¼�
    (*tmp).data.fd = *linkfd;//�����¼���Ӧ���ļ�������
    if(epoll_ctl(*epfd,EPOLL_CTL_ADD,*linkfd,tmp))//��epoll�����һ������linkfd�ļ����¼�
    {
        sys_err("epoll_ctl()1");
    }
}

void cloud_info_trans_process(char *buffer,FILE *fp,int shmid,int *pipefd)
{

    if(fgets(buffer, BUFFER_SIZE, fp) != NULL)
    {
        printf("rec:%s\n", buffer);
        if(strncmp(buffer,"received a message",18)==0)
        {
            printf("in\n");
            //������
            char *search_str = "\"value\"";
            char *temp_char = buffer;
            char per_info[20] ;
            char trans_info[120];
            char *ptr=NULL;
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
            printf("the data:%s\n",trans_info);
            if(strcmp(ptr,"")!=0)//��Ϊ��
            {
                memset(ptr,0,MEMSIZE);//��Ϊ�������
            }
            strcpy(ptr,trans_info);
            //�����źŸ�������
            //kill(getppid(), SIGUSR1);
            //close(pipefd[0]); // �رն���
            write(pipefd[1], "data_update", 12); // ��ܵ�д����,����ᴥ�������������̵ĸ������ݲ���
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
     //�ͻ��������߼�
    //printf("client_set_info\n");
    //char *sonic_s,*buzzer_s,*led_s,*door_s,*fan_s;
    char *rec_s_array[5];
    //int8_t upload_state[5];
    //��ô������Ϊstrtok���޸�ԭ�ַ��������ָ�����Ϊ��������ʵ�֣�����ÿ��ֻ�����µ�������strtok
    char *temp_info1 = (char *)malloc(strlen(info)+1);
    strcpy(temp_info1,info);
    strtok(temp_info1,":");
    for(int i=0;i<5;i++)
    {
        rec_s_array[i]=strtok(NULL,":");
    }

    //��֯����
    char *pub_topic = "/"PRODUCT_KEY"/"DEVICE_NAME"/user/rk3568_set_info";
    char temp_set_char[100];
    //printf("info:%s\n",rec_s_array[4]);
    sprintf(temp_set_char,"{\"s_s\": %d, \"b_s\": %d, \"l_s\": %d, \"d_s\": %d, \"m_s\": %d, \"TargetDevice\": \"mqtt_stm32\"}",check_state(rec_s_array[0]),check_state(rec_s_array[1]),check_state(rec_s_array[2]),check_state(rec_s_array[3]),check_state(rec_s_array[4]));
    aiot_msg_t *pub_message = aiot_msg_create_raw(pub_topic, (uint8_t *)temp_set_char, strlen(temp_set_char));

    /* ������?? */
    aiot_device_send_message(device_client, pub_message);

    /* ɾ����Ϣ */
    aiot_msg_delete(pub_message);
    //���ͼ���
    printf("send set to aliyun successfully\n");
}

void case2_request_data_process(struct epoll_event *events,int i,char *ptr)
{   
    char trans_info[100];
    if(strcmp(ptr,"")==0) //�����ptr�ǽ����ӽ��̽���aliyun���ݵĹ����ڴ�����
    {
        sprintf(trans_info,"empty_data");
        if(send(events[i].data.fd,trans_info,strlen(trans_info)+1,0)<0)
        {
            perror("fail to send sqlinfo\n");
            return;
            //break;//����switch���棬for������
        }
        printf("empty data\n");
    }
    else
    {
        sprintf(trans_info,"return_data:%s",ptr);
        //strncpy(trans_info,ptr,strlen(ptr)+1);
        //printf("trans_info:%s\n",trans_info);
        if(send(events[i].data.fd,trans_info,strlen(trans_info)+1,0)<0)
        {
            perror("fail to send sqlinfo\n");
            return;
            //break;//����switch���棬for������
        }
        printf("trans_info successfully\n");
    }
   
}


void case1_goverment_delete_process(char *info,MYSQL *mysql,struct epoll_event *events,int i)
{
    char *name;
    //��ô������Ϊstrtok���޸�ԭ�ַ��������ָ�����Ϊ��������ʵ�֣�����ÿ��ֻ�����µ�������strtok
    char *temp_info1 = (char *)malloc(strlen(info)+1);
    strcpy(temp_info1,info);
    name = strtok(temp_info1,":");
    if(c_db_select(mysql,"SELECT * FROM login_info",name,NULL,NULL)==2)
    {
        printf("find\n");
        char delete_sql[50];
        sprintf(delete_sql,"delete from login_info where name = '%s'",name);
        c_db_delete(mysql,delete_sql);
        
        //�������ݿ�
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



//���������Ī������δ���
void case1_goverment_change_process(char *info,MYSQL *mysql,struct epoll_event *events,int i)
{
    printf("\n");
    char *name,*id,*account,*password;
    //��ô������Ϊstrtok���޸�ԭ�ַ��������ָ�����Ϊ��������ʵ�֣�����ÿ��ֻ�����µ�������strtok
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
        //�����ݿ�����޸�
        c_db_update(mysql,update_sql);

        //�������ݿ�
        char *sendinfo = c_db_get(mysql,"SELECT * FROM login_info");
        char *new_sendinfo = (char *)malloc(strlen(sendinfo)+7);
        sprintf(new_sendinfo,"manage%s\n",sendinfo);
        
        //printf("%s%d\n",new_sendinfo,strlen(new_sendinfo));
        if(send(events[i].data.fd,new_sendinfo,strlen(new_sendinfo)+1,0)<0)
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
        send(events[i].data.fd,"goverment:no",13,0);
        //printf("goverment:no\n");
    }
    //�ͷ�
    if(temp_info1!=NULL)
    {
        free(temp_info1);
    }
    
}


void case1_goverment_login_process(MYSQL *mysql,struct epoll_event *events,int i)
{
    char *sendinfo = c_db_get(mysql,"SELECT * FROM login_info");
    char *new_sendinfo = (char *)malloc(strlen(sendinfo)+7);
    sprintf(new_sendinfo,"manage%s\n",sendinfo);
    //printf("%s%d\n",new_sendinfo,strlen(new_sendinfo));
    send(events[i].data.fd,new_sendinfo,strlen(new_sendinfo)+1,0);
    free(new_sendinfo);
    free(sendinfo);
}


void case1_register_process(char *info,MYSQL *mysql,struct epoll_event *events,int i)
{
    //ͬΪ�ֲ�����
    char *name,*account,*password;
    //��ô������Ϊstrtok���޸�ԭ�ַ��������ָ�����Ϊ��������ʵ�֣�����ÿ��ֻ�����µ�������strtok
    char *temp_info1 = (char *)malloc(strlen(info)+1);
    strcpy(temp_info1,info);
    name = strtok(temp_info1,":");
    account = strtok(NULL,":");
    password = strtok(NULL,":");
    char select_sql[200];
    //�����idʹ��������䣬�Ҳ��ظ�
    sprintf(select_sql,"insert login_info(name,id,account,password) values('%s',FLOOR(RAND() * (999999 - 100000 + 1)) + 100000,'%s','%s')",name,account,password);//�������
    
    if(c_db_insert(mysql,select_sql)==0)
    {
        //printf("registe success\n");
        send(events[i].data.fd,"register:yes",13,0);
    }
    else
    {
        //printf("registe failed\n");
        send(events[i].data.fd,"register:no",12,0);
    }
    //�ͷ�
    free(temp_info1);
}

void case1_login_process(char *info,MYSQL *mysql,struct epoll_event *events,int i)
{
    //��strtok��ص�ָ�붼�����ͷţ�������ԭ�ַ��������ϵõ��ģ�ֻҪԭʼ�ַ�������malloc�ľͲ����ͷ�
    char *name,*account,*password;
    //��ô������Ϊstrtok���޸�ԭ�ַ��������ָ�����Ϊ��������ʵ�֣�����ÿ��ֻ�����µ�������strtok
    char *temp_info1 = (char *)malloc(strlen(info)+1);
    strcpy(temp_info1,info);
    name = strtok(temp_info1,":");
    account = strtok(NULL,":");
    password = strtok(NULL,":");
    //��ȡ���ݿ����Ϣ
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
    //�����ͷţ���ֹй¶
    free(temp_info1);
}




char *get_info_tag(char *info)
{
    char *token;
    char *lasttoken;
    char *last_trans_token = (char *)malloc(strlen(info)+1);
    char *temp_info = (char *)malloc(strlen(info)+1);
    strcpy(temp_info,info);
    
    token = strtok(temp_info,":");
    while(token!=NULL)
    {
        lasttoken = token;
        token = strtok(NULL,":");
        //printf("token:%s\n",token);                                                        
    }
    strcpy(last_trans_token,lasttoken);
    free(temp_info);
    return last_trans_token;
}

//tips:shift+tabʹѡ�еĶ�����ǰ����
int get_server_state(char *info)
{
    static int8_t old_state=0;
    char *temp_info_0 = (char *)malloc(strlen(info)+1);//���������벢��ʹ�ã���Ӱ��ԭ�����ַ���
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
            printf("wrong state change info\n");
        }
    }
    else
    {
        free(temp_info_0);
        return -1;
    }
    free(temp_info_0);
    return old_state;

}


void *init_rk_to_aliyun()
{
    //mqtt���Ӱ����Ƶĳ�ʼ������
    int32_t     res = STATE_SUCCESS;
    /* ����SDK�ĵײ����� */
    aiot_sysdep_init(aiot_sysdep_get_portfile());

    /* �����豸 */
    void *device_client = aiot_device_create(PRODUCT_KEY, DEVICE_NAME);
    if (device_client == NULL) {
        printf("device_client failed\n");
        exit(1);
    }

    /* �����豸��Կ */
    aiot_device_set_device_secret(device_client, DEVICE_SECRET);
    aiot_linkconfig_t* config = aiot_linkconfig_init(MQTT_PROTOCOL);
    /* ���÷�������host��port */
    aiot_linkconfig_host(config, HOST, PORT1);

    /* �����豸���Ӳ��� */
    aiot_device_set_linkconfig(device_client, config);
    /* �豸���� */
    res = aiot_device_connect(device_client);
    if (res < STATE_SUCCESS) {
        /* ���Խ�������ʧ��, ����MQTTʵ��, ������Դ */
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

//fun_process_client����,�������Կͻ��˵���������ƿͻ��˵�����
//gcc rk3568_server.c connect_mysql.c -o rk3568_server -L/user/lib/mysql -lmysqlclient
