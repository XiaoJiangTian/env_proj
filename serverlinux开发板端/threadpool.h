#ifndef __THREADPOOL_H
#define __THREADPOOL_H

//�����̳߳أ����ҳ�ʼ��һ���������̵߳ȴ�����
struct pool *thread_pool_create(int min_thr,int max_thr,int queue_max);

//�·��������������У��̳߳�ͨ������������һ���̶߳���������е�������д���
int thread_pool_addclient(struct pool*tmp_pool,void*(*funtion)(void *arg),void *arg);

//�������߳̽����ͷš����һ����ڴ���Դ
int thread_pool_destory(struct pool *tmp_pool);
#endif