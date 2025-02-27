#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include<sys/ipc.h>
#include<sys/shm.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#define MEMSIZE 1024

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
    int shmid;
    pid_t pid;
    char *ptr;//共享内存区域

    shmid = shmget(IPC_PRIVATE,MEMSIZE,0664);//获取shmid
    if(shmid<0)
    {
        perror("fail to shmget\n");
        exit(1);
    }

    pid = fork();
    if(pid<0)
    {
        perror("fail to fork\n");
        exit(1);
    }
    else if(pid == 0)//子进程
    {
        while(1)
        {
            sleep(1);
            char *str="received a message \"{\"deviceType\":\"CustomCategory\",\"iotId\":\"oW44zFBKdNwyTiCv5p8Gk1m420\",\"requestId\":\"null\",\"checkFailedData\":{},\"productKey\":\"k1m42D6OwDF\",\"gmtCreate\":1722348908726,\"deviceName\":\"mqtt_stm32\", \
                \"items\":{\"rain\":{\"value\":0,\"time\":1722348908722},\"led_state\":{\"value\":1,\"time\":1722348908722},\"dht11_temper\":{\"value\":29.3,\"time\":1722348908722},\"door_state\":{\"value\":0,\"time\":1722348908722}, \
                \"motor_state\":{\"value\":0,\"time\":1722348908722},\"dht11_huminity\":{\"value\":51.0,\"time\":1722348908722}}}\"";
            char *search_str = "\"value\"";
            
            char *temp_char = str;
            char per_info[20] ;
            char trans_info[120];
            while ((temp_char=strstr(temp_char,search_str))!=NULL)
            {
                //printf("str:%s\n",temp_char);
                
                char *extract_str = extract_between(temp_char,':',',');
                printf("extract str:%s\n",extract_str);
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
            //shmdt(ptr);
        }
        
        exit(0);
    
    }
    else//父进程
    {
        while(1)
        {
            sleep(2);
            //wait(NULL);//子进程先运行
            if((ptr = shmat(shmid,NULL,0))==(void *)-1)
            {
                perror("fail to shmat\n");
                exit(1);
            }
            printf("from son process:%s\n",ptr);
            // shmdt(ptr);//接触映射
            // //去除shmid
            // if(shmctl(shmid,IPC_RMID,0)<0)
            // {
            //     perror("fail to shmctl\n");
            //     exit(1);
            // }
        }
        
        exit(0);
    }
    exit(0);
    // char *str="received a message \"{\"deviceType\":\"CustomCategory\",\"iotId\":\"oW44zFBKdNwyTiCv5p8Gk1m420\",\"requestId\":\"null\",\"checkFailedData\":{},\"productKey\":\"k1m42D6OwDF\",\"gmtCreate\":1722348908726,\"deviceName\":\"mqtt_stm32\", \
    //             \"items\":{\"rain\":{\"value\":0,\"time\":1722348908722},\"led_state\":{\"value\":1,\"time\":1722348908722},\"dht11_temper\":{\"value\":29.3,\"time\":1722348908722},\"door_state\":{\"value\":0,\"time\":1722348908722}, \
    //             \"motor_state\":{\"value\":0,\"time\":1722348908722},\"dht11_huminity\":{\"value\":51.0,\"time\":1722348908722}}}\"";
    // char *search_str = "\"value\"";
    
    // char *temp_char = str;
    // while ((temp_char=strstr(temp_char,search_str))!=NULL)
    // {
    //     //printf("str:%s\n",temp_char);
        
    //     char *extract_str = extract_between(temp_char,':',',');
    //     printf("extract str:%s\n",extract_str);
    //     temp_char += strlen(search_str);
    // }
    
    

    

    //printf("%s\n",strstr(str,search_str));
    return 0;
}
