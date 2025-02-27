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

//线程池结构体
struct pool{
    //锁
    pthread_mutex_t pool_mutex;//锁pool
    pthread_mutex_t busythread_num_mutex;//忙线程个数

    //条件变量
    pthread_cond_t queue_not_full; //任务队列满时等待此信号量满足，即等待不满的情况出现
    pthread_cond_t queue_not_empty;//队伍为空时阻塞等待

    pthread_t *thread_pool;//线程池指针
    pthread_t thread_pool_manage;//管理线程大小的线程
    thread_task_queue *task_queue;//待处理的任务队列


    //这些参数用于后续的动态线程池参数调整
    //线程池的相关参数
    int min_thr_num;//最小线程数
    int max_thr_num;//最大线程数
    int live_thr_num;//存活线程数
    int busy_thr_num;//忙线程数
    int wait_exitthr_num;//等待销毁线程数

    //任务队列的相关参数
    int queue_front;//任务队列前端索引
    int queue_tail;//任务队列后端索引
    int queue_size;//任务队列的实际大小
    int queue_maxsize;
    
    bool shutdown;//标志位，线程池使用标志位
};

//释放包括任务队列空间，线程池空间，线程结构体空间， 锁，条件变量
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
        free(tmp_pool->thread_pool);//写最后面以防段错误，即内存已被释放还在使用
        pthread_mutex_lock(&(tmp_pool->pool_mutex));
        pthread_mutex_destroy(&(tmp_pool->pool_mutex));
        pthread_mutex_lock(&(tmp_pool->busythread_num_mutex));
        pthread_mutex_destroy(&(tmp_pool->busythread_num_mutex));
        pthread_cond_destroy(&(tmp_pool->queue_not_empty));
        pthread_cond_destroy(&(tmp_pool->queue_not_full));
     
    }
    free(tmp_pool);
    tmp_pool=NULL;//将线程池结构体释放，并置null防止野指针
    return 0;
}


void *pthreadcreate(void *s)
{
    struct pool *tmp=(struct pool*)s;
    thread_task_queue task;
    printf("pthread_create\n");
    while (1)
    {
        //对pool结构体内的元素进行访问时要加锁，因为是多线程
        pthread_mutex_lock(&(tmp->pool_mutex));
        while ((tmp->queue_size==0) && (!tmp->shutdown))
        {
            printf("thread is waiting for task!\n");
            //等待这个条件变量
            pthread_cond_wait(&(tmp->queue_not_empty),&(tmp->pool_mutex));
            //printf("wait1\n");
            if(tmp->wait_exitthr_num>0)
            {
                tmp->wait_exitthr_num--;
                //如果还大于最小线程要求自然会减，如果不满足就不会再减，并且让wait_exitthr_num的数量自减到0
                if(tmp->live_thr_num > tmp->min_thr_num)
                {
                    printf("thread 0x%x is exiting\n",(unsigned int)pthread_self());
                    tmp->live_thr_num--;
                    pthread_mutex_unlock(&(tmp->pool_mutex));

                    pthread_exit(NULL);
                }
            }
        }

        //到队列不再为空时，会退出上面的while循环
        if(tmp->shutdown)
        {
            pthread_mutex_unlock(&(tmp->pool_mutex));
            printf("thread 0x%x is exiting!\n",(unsigned int)pthread_self());
            //线程分离态，线程自行退出，不需主线程阻塞等待
            pthread_detach(pthread_self()); //这里的分离退出是因为不涉及主线程退出，后面的全体退出，是全部线程配合一起退出
            pthread_exit(NULL);
        }

        //如果不是线程想要退出就是，那么就执行任务队列的任务
        task.function = tmp->task_queue[tmp->queue_front].function;
        task.arg = tmp->task_queue[tmp->queue_front].arg;
        
        //对所需执行任务进行出队
        tmp->queue_front = (tmp->queue_front+1) % (tmp->queue_maxsize);
        tmp->queue_size--;
        
        //发出任务队列非满的信号
        pthread_cond_broadcast(&(tmp->queue_not_full));

        //对pool结构体的操作完成
        pthread_mutex_unlock(&(tmp->pool_mutex));

        //正式准备执行任务
        printf("thread 0x%x start working\n",(unsigned int)pthread_self());

        //加锁并且修改忙线程数
        pthread_mutex_lock(&(tmp->busythread_num_mutex));
        tmp->busy_thr_num++;
        pthread_mutex_unlock(&(tmp->busythread_num_mutex));
        
        (*task.function)(task.arg);//执行函数
        //就是这个函数就相当于1个线程，此时该线程抢到后，就会执行函数。
        //对忙线程-1，主要是用于管理线程动态调整线程池
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
    //线程池处于运行状态时
    while(!tmp_pool->shutdown)
    {
        printf("manage_thread\n");
        //管理线程定时执行
        sleep(DEFAULT_TIME);


        //获取pool相关的结构体
        pthread_mutex_lock(&(tmp_pool->pool_mutex));
        int queue_size = tmp_pool->queue_size;
        int live_thr = tmp_pool->live_thr_num;
        pthread_mutex_unlock(&(tmp_pool->pool_mutex));

        pthread_mutex_lock(&(tmp_pool->busythread_num_mutex));
        int busy_thr = tmp_pool->busy_thr_num;
        pthread_mutex_unlock(&(tmp_pool->busythread_num_mutex));

        //当任务队列数大于最小线程数，存在线程数小于最大线程数时增加线程
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

        //减少线程：当存活线程大于忙线程两倍时减少，前提是要大于最小线程限制
        if((busy_thr*2)<live_thr && live_thr>tmp_pool->min_thr_num)
        {
            pthread_mutex_lock(&(tmp_pool->pool_mutex));
            tmp_pool->wait_exitthr_num = DEFAULT_THREAD;
            pthread_mutex_unlock(&(tmp_pool->pool_mutex));
            for(i=0;i<DEFAULT_THREAD;i++)
            {
                pthread_cond_signal(&(tmp_pool->queue_not_empty));//假唤醒线程，实际退出
            }
        }

    }
    return NULL;
}

//应用到我这个项目里面就是，客户端连进来，相当于任务队列添加一项，交给一个线程去处理，各个线程去抢
int thread_pool_addclient(struct pool*tmp_pool,void*(*funtion)(void *arg),void *arg)
{
    pthread_mutex_lock(&(tmp_pool->pool_mutex));
    if((tmp_pool->live_thr_num==tmp_pool->max_thr_num) && (!tmp_pool->shutdown))
    {
        pthread_cond_wait(&(tmp_pool->queue_not_full),&(tmp_pool->pool_mutex));//等待队列非满的情况
    }
    if(tmp_pool->shutdown)
    {
        pthread_cond_broadcast(&(tmp_pool->queue_not_empty));//假唤醒实退出，在线程的程序里
        //退出前加锁了话一定记得解锁
        pthread_mutex_unlock(&(tmp_pool->pool_mutex));
        return 0; 
    }

    //正式开始往任务队列里添加任务
    if(tmp_pool->task_queue[tmp_pool->queue_tail].arg!=NULL)
    {
        tmp_pool->task_queue[tmp_pool->queue_tail].arg=NULL;
    }
    //传递任务队列所需的函数，以及参数
    tmp_pool->task_queue[tmp_pool->queue_front].function = funtion;
    tmp_pool->task_queue[tmp_pool->queue_front].arg = arg;
    //队列，从尾部添加任务，从头部取任务
    tmp_pool->queue_tail = (tmp_pool->queue_tail+1) % tmp_pool->queue_maxsize;
    tmp_pool->queue_size++;

    //发出条件变量的通知，各个线程之前的运行协同就靠这个
    pthread_cond_signal(&(tmp_pool->queue_not_empty)); //线程池里线程会收到信号
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
        //这里应该申请的大小是结构体大小，不能用结构体指针去申请，申请的是一个指针的大小
        if((thr_pool=(struct pool *)malloc(sizeof(struct pool)))==NULL)
        {
            perror("fail to malloc pool");
            break;
        }

        //初始化
        thr_pool->min_thr_num = min_thr; //最小线程数
        thr_pool->max_thr_num = max_thr; //最大线程数
        thr_pool->live_thr_num = min_thr; 
        thr_pool->queue_maxsize = queue_max; //任务队列的最大值
        thr_pool->busy_thr_num = 0;
        thr_pool->queue_front = 0;
        thr_pool->queue_tail = 0;
        thr_pool->queue_size = 0;
        thr_pool->shutdown = 0;

        //按最大线程数为线程池申请空间
        if((thr_pool->thread_pool = (pthread_t*)malloc(sizeof(pthread_t) * thr_pool->max_thr_num))==NULL)
        {
            perror("fail to malloc pthread_t");
            break;
        }

        //这里相对原代码有变化，原代码是thr_pool
        

        //memset(thr_pool,0,sizeof(struct pool)*(thr_pool->max_thr_num));
        //memset(thr_pool->thread_pool,0,sizeof(pthread_t)*(thr_pool->max_thr_num));//对申请到的线程池结构体空间置0
        for(i=0;i<thr_pool->max_thr_num;i++)
        {
            thr_pool->thread_pool[i]=0;//用于判断该线程是否使用
        }

        //初始化任务队列
        if((thr_pool->task_queue = (thread_task_queue*)malloc(sizeof(thread_task_queue) * thr_pool->queue_maxsize))==NULL)
        {
            perror("fail to malloc thread_task_queue");
            break;
        }

        //初始化条件变量和互斥锁
        if(pthread_mutex_init(&thr_pool->pool_mutex,NULL)!=0 \
        ||pthread_mutex_init(&thr_pool->busythread_num_mutex,NULL)!=0 \
        ||pthread_cond_init(&thr_pool->queue_not_empty,NULL)!=0 \
        ||pthread_cond_init(&thr_pool->queue_not_full,NULL)!=0)
        {
            perror("fail to init cond or mutex");
            break;
        }
        //初始化最低要求的线程
        for(i=0;i<min_thr;i++)
        {
            pthread_create(&(thr_pool->thread_pool[i]),NULL,pthreadcreate,(void *)thr_pool);
            printf("pthread create for :%d\n",(unsigned int)thr_pool->thread_pool[i]);//打印新建的线程号
        }

        //创建管理线程
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


int thread_pool_destory(struct pool *tmp_pool)//销毁线程
{
    int i;
    
    if(tmp_pool==NULL)
    {
        return -1;
    }
    tmp_pool->shutdown = 1;
    //等待回收管理线程
    pthread_join(tmp_pool->thread_pool_manage,NULL);

    //使用条件变量机制关闭线程池中的线程
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