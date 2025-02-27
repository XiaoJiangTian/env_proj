#ifndef __THREADPOOL_H
#define __THREADPOOL_H

//创建线程池，并且初始化一定数量的线程等待任务
struct pool *thread_pool_create(int min_thr,int max_thr,int queue_max);

//下放任务进入任务队列，线程池通过抢，让其中一个线程对任务队列中的任务进行处理
int thread_pool_addclient(struct pool*tmp_pool,void*(*funtion)(void *arg),void *arg);

//对所有线程进行释放、并且回收内存资源
int thread_pool_destory(struct pool *tmp_pool);
#endif