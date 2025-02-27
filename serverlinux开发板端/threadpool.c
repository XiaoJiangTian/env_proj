#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<string.h>
#include<unistd.h>
#include<stdbool.h>
#include "threadpool.h"

#define DEFAULT_TIME 5
#define DEFAULT_THREAD 10

// static void sys_err(char *s)
// {
//     perror(s);
// }

typedef struct{
    void *(*function)(void *);
    void *arg;
}thread_task_queue;

//�̳߳ؽṹ��
struct pool{
    //��
    pthread_mutex_t pool_mutex;//��pool
    pthread_mutex_t busythread_num_mutex;//æ�̸߳���

    //��������
    pthread_cond_t queue_not_full; //���������ʱ�ȴ����ź������㣬���ȴ��������������
    pthread_cond_t queue_not_empty;//����Ϊ��ʱ�����ȴ�

    pthread_t *thread_pool;//�̳߳�ָ��
    pthread_t thread_pool_manage;//�����̴߳�С���߳�
    thread_task_queue *task_queue;//��������������


    //��Щ�������ں����Ķ�̬�̳߳ز�������
    //�̳߳ص���ز���
    int min_thr_num;//��С�߳���
    int max_thr_num;//����߳���
    int live_thr_num;//����߳���
    int busy_thr_num;//æ�߳���
    int wait_exitthr_num;//�ȴ������߳���

    //������е���ز���
    int queue_front;//�������ǰ������
    int queue_tail;//������к������
    int queue_size;//������е�ʵ�ʴ�С
    int queue_maxsize;
    
    bool shutdown;//��־λ���̳߳�ʹ�ñ�־λ
};

//�ͷŰ���������пռ䣬�̳߳ؿռ䣬�߳̽ṹ��ռ䣬 ������������
int thread_pool_free(struct pool * tmp_pool)
{
    if(tmp_pool==NULL)
    {
        return  -1;
    }
    if(tmp_pool->task_queue)
    {
        printf("errno1\n");
        free(tmp_pool->task_queue);
    }
    if(tmp_pool->thread_pool)
    {
        printf("errno2\n");
        free(tmp_pool->thread_pool);//д������Է��δ��󣬼��ڴ��ѱ��ͷŻ���ʹ��
        pthread_mutex_lock(&(tmp_pool->pool_mutex));
        pthread_mutex_destroy(&(tmp_pool->pool_mutex));
        pthread_mutex_lock(&(tmp_pool->busythread_num_mutex));
        pthread_mutex_destroy(&(tmp_pool->busythread_num_mutex));
        pthread_cond_destroy(&(tmp_pool->queue_not_empty));
        pthread_cond_destroy(&(tmp_pool->queue_not_full));
     
    }
    free(tmp_pool);
    tmp_pool=NULL;//���̳߳ؽṹ���ͷţ�����null��ֹҰָ��
    return 0;
}


void *pthreadcreate(void *s)
{
    struct pool *tmp=(struct pool*)s;
    thread_task_queue task;
    printf("pthread_create\n");
    while (1)
    {
        //��pool�ṹ���ڵ�Ԫ�ؽ��з���ʱҪ��������Ϊ�Ƕ��߳�
        pthread_mutex_lock(&(tmp->pool_mutex));
        while ((tmp->queue_size==0) && (!tmp->shutdown))
        {
            printf("thread is waiting for task!\n");
            //�ȴ������������
            pthread_cond_wait(&(tmp->queue_not_empty),&(tmp->pool_mutex));
            //printf("wait1\n");
            if(tmp->wait_exitthr_num>0)
            {
                tmp->wait_exitthr_num--;
                //�����������С�߳�Ҫ����Ȼ��������������Ͳ����ټ���������wait_exitthr_num�������Լ���0
                if(tmp->live_thr_num > tmp->min_thr_num)
                {
                    printf("thread 0x%x is exiting\n",(unsigned int)pthread_self());
                    tmp->live_thr_num--;
                    pthread_mutex_unlock(&(tmp->pool_mutex));

                    pthread_exit(NULL);
                }
            }
        }

        //�����в���Ϊ��ʱ�����˳������whileѭ��
        if(tmp->shutdown)
        {
            pthread_mutex_unlock(&(tmp->pool_mutex));
            printf("thread 0x%x is exiting!\n",(unsigned int)pthread_self());
            //�̷߳���̬���߳������˳����������߳������ȴ�
            pthread_detach(pthread_self()); //����ķ����˳�����Ϊ���漰���߳��˳��������ȫ���˳�����ȫ���߳����һ���˳�
            pthread_exit(NULL);
        }

        //��������߳���Ҫ�˳����ǣ���ô��ִ��������е�����
        task.function = tmp->task_queue[tmp->queue_front].function;
        task.arg = tmp->task_queue[tmp->queue_front].arg;
        
        //������ִ��������г���
        tmp->queue_front = (tmp->queue_front+1) % (tmp->queue_maxsize);
        tmp->queue_size--;
        
        //����������з������ź�
        pthread_cond_broadcast(&(tmp->queue_not_full));

        //��pool�ṹ��Ĳ������
        pthread_mutex_unlock(&(tmp->pool_mutex));

        //��ʽ׼��ִ������
        printf("thread 0x%x start working\n",(unsigned int)pthread_self());

        //���������޸�æ�߳���
        pthread_mutex_lock(&(tmp->busythread_num_mutex));
        tmp->busy_thr_num++;
        pthread_mutex_unlock(&(tmp->busythread_num_mutex));
        
        (*task.function)(task.arg);//ִ�к���
        //��������������൱��1���̣߳���ʱ���߳������󣬾ͻ�ִ�к�����
        //��æ�߳�-1����Ҫ�����ڹ����̶߳�̬�����̳߳�
        pthread_mutex_lock(&(tmp->busythread_num_mutex));
        tmp->busy_thr_num--;
        pthread_mutex_unlock(&(tmp->busythread_num_mutex));
    }
    pthread_exit(NULL);
}

bool is_thread_alive(pthread_t pid)
{
    return pid>0?1:0;
}
void *pthreadmanage(void *pool)
{
    int i;
    struct pool *tmp_pool = (struct pool*)pool;

    printf("pthread_manage_create\n");
    //�̳߳ش�������״̬ʱ
    while(!tmp_pool->shutdown)
    {
        printf("manage_thread\n");
        //�����̶߳�ʱִ��
        sleep(DEFAULT_TIME);


        //��ȡpool��صĽṹ��
        pthread_mutex_lock(&(tmp_pool->pool_mutex));
        int queue_size = tmp_pool->queue_size;
        int live_thr = tmp_pool->live_thr_num;
        pthread_mutex_unlock(&(tmp_pool->pool_mutex));

        pthread_mutex_lock(&(tmp_pool->busythread_num_mutex));
        int busy_thr = tmp_pool->busy_thr_num;
        pthread_mutex_unlock(&(tmp_pool->busythread_num_mutex));

        //�����������������С�߳����������߳���С������߳���ʱ�����߳�
        if(queue_size>=tmp_pool->min_thr_num && live_thr<tmp_pool->max_thr_num)
        {
            pthread_mutex_lock(&(tmp_pool->pool_mutex));
            int add =0;
            for(i=0;i<tmp_pool->max_thr_num && add<DEFAULT_THREAD && tmp_pool->live_thr_num<tmp_pool->max_thr_num;i++)
            {
                if(tmp_pool->thread_pool[i]==0 || !is_thread_alive(tmp_pool->thread_pool[i]))
                {
                    pthread_create(&(tmp_pool->thread_pool[i]),NULL,pthreadcreate,(void *)tmp_pool);
                    add++;
                    tmp_pool->live_thr_num++;
                }
            }
        }

        //�����̣߳�������̴߳���æ�߳�����ʱ���٣�ǰ����Ҫ������С�߳�����
        if((busy_thr*2)<live_thr && live_thr>tmp_pool->min_thr_num)
        {
            pthread_mutex_lock(&(tmp_pool->pool_mutex));
            tmp_pool->wait_exitthr_num = DEFAULT_THREAD;
            pthread_mutex_unlock(&(tmp_pool->pool_mutex));
            for(i=0;i<DEFAULT_THREAD;i++)
            {
                pthread_cond_signal(&(tmp_pool->queue_not_empty));//�ٻ����̣߳�ʵ���˳�
            }
        }

    }
    return NULL;
}

//Ӧ�õ��������Ŀ������ǣ��ͻ������������൱������������һ�����һ���߳�ȥ���������߳�ȥ��
int thread_pool_addclient(struct pool*tmp_pool,void*(*funtion)(void *arg),void *arg)
{
    pthread_mutex_lock(&(tmp_pool->pool_mutex));
    if((tmp_pool->live_thr_num==tmp_pool->max_thr_num) && (!tmp_pool->shutdown))
    {
        pthread_cond_wait(&(tmp_pool->queue_not_full),&(tmp_pool->pool_mutex));//�ȴ����з��������
    }
    if(tmp_pool->shutdown)
    {
        pthread_cond_broadcast(&(tmp_pool->queue_not_empty));//�ٻ���ʵ�˳������̵߳ĳ�����
        //�˳�ǰ�����˻�һ���ǵý���
        pthread_mutex_unlock(&(tmp_pool->pool_mutex));
        return 0; 
    }

    //��ʽ��ʼ������������������
    if(tmp_pool->task_queue[tmp_pool->queue_tail].arg!=NULL)
    {
        tmp_pool->task_queue[tmp_pool->queue_tail].arg=NULL;
    }
    //���������������ĺ������Լ�����
    tmp_pool->task_queue[tmp_pool->queue_front].function = funtion;
    tmp_pool->task_queue[tmp_pool->queue_front].arg = arg;
    //���У���β��������񣬴�ͷ��ȡ����
    tmp_pool->queue_tail = (tmp_pool->queue_tail+1) % tmp_pool->queue_maxsize;
    tmp_pool->queue_size++;

    //��������������֪ͨ�������߳�֮ǰ������Эͬ�Ϳ����
    pthread_cond_signal(&(tmp_pool->queue_not_empty)); //�̳߳����̻߳��յ��ź�
    pthread_mutex_unlock(&(tmp_pool->pool_mutex));

    return 0;
}

struct pool *thread_pool_create(int min_thr,int max_thr,int queue_max)
{
    int i,ret_num;
    struct pool *thr_pool=NULL;
    do
    {
        printf("thread_create\n");
        //����Ӧ������Ĵ�С�ǽṹ���С�������ýṹ��ָ��ȥ���룬�������һ��ָ��Ĵ�С
        if((thr_pool=(struct pool *)malloc(sizeof(struct pool)))==NULL)
        {
            perror("fail to malloc pool");
            break;
        }

        //��ʼ��
        thr_pool->min_thr_num = min_thr; //��С�߳���
        thr_pool->max_thr_num = max_thr; //����߳���
        thr_pool->live_thr_num = min_thr; 
        thr_pool->queue_maxsize = queue_max; //������е����ֵ
        thr_pool->busy_thr_num = 0;
        thr_pool->queue_front = 0;
        thr_pool->queue_tail = 0;
        thr_pool->queue_size = 0;
        thr_pool->shutdown = 0;

        //������߳���Ϊ�̳߳�����ռ�
        if((thr_pool->thread_pool = (pthread_t*)malloc(sizeof(pthread_t) * thr_pool->max_thr_num))==NULL)
        {
            perror("fail to malloc pthread_t");
            break;
        }

        //�������ԭ�����б仯��ԭ������thr_pool
        

        //memset(thr_pool,0,sizeof(struct pool)*(thr_pool->max_thr_num));
        //memset(thr_pool->thread_pool,0,sizeof(pthread_t)*(thr_pool->max_thr_num));//�����뵽���̳߳ؽṹ��ռ���0
        for(i=0;i<thr_pool->max_thr_num;i++)
        {
            thr_pool->thread_pool[i]=0;//�����жϸ��߳��Ƿ�ʹ��
        }

        //��ʼ���������
        if((thr_pool->task_queue = (thread_task_queue*)malloc(sizeof(thread_task_queue) * thr_pool->queue_maxsize))==NULL)
        {
            perror("fail to malloc thread_task_queue");
            break;
        }

        //��ʼ�����������ͻ�����
        if(pthread_mutex_init(&thr_pool->pool_mutex,NULL)!=0 \
        ||pthread_mutex_init(&thr_pool->busythread_num_mutex,NULL)!=0 \
        ||pthread_cond_init(&thr_pool->queue_not_empty,NULL)!=0 \
        ||pthread_cond_init(&thr_pool->queue_not_full,NULL)!=0)
        {
            perror("fail to init cond or mutex");
            break;
        }
        //��ʼ�����Ҫ����߳�
        for(i=0;i<min_thr;i++)
        {
            pthread_create(&(thr_pool->thread_pool[i]),NULL,pthreadcreate,(void *)thr_pool);
            printf("pthread create for :%d\n",(unsigned int)thr_pool->thread_pool[i]);//��ӡ�½����̺߳�
        }

        //���������߳�
        if(ret_num = pthread_create(&(thr_pool->thread_pool_manage),NULL,pthreadmanage,(void *)thr_pool))
        {
            perror(strerror(ret_num));
            exit(1);
        }
        return thr_pool;

    } while (0);
    
    thread_pool_free(thr_pool);

    return NULL;
}


int thread_pool_destory(struct pool *tmp_pool)//�����߳�
{
    int i;
    
    if(tmp_pool==NULL)
    {
        return -1;
    }
    tmp_pool->shutdown = 1;
    //�ȴ����չ����߳�
    pthread_join(tmp_pool->thread_pool_manage,NULL);

    //ʹ�������������ƹر��̳߳��е��߳�
    for(i=0;i<tmp_pool->live_thr_num;i++)
    {
        pthread_cond_broadcast(&(tmp_pool->queue_not_empty));
    }

    for(i=0;i<tmp_pool->live_thr_num;i++)
    {
        pthread_join((tmp_pool->thread_pool[i]),NULL);
    }
    thread_pool_free(tmp_pool);
    return 0;
}