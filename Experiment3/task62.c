#include <pthread.h>
#include "wrapper.h"

void *increase(void *arg);
void *decrease(void *arg);
int cnt = 0;
sem_t mutex;    //Pthreads信号量

int main(int argc, char **argv)
{
    unsigned int niters;
    pthread_t tid1, tid2;

    sem_init(&mutex, 0, 1);     //初始化Pthreads信号量mutex，初始化其value为1

    if(argc != 2){  //检查命令行参数的合法性
        printf("usage:%s <niters>\n", argv[0]);
        exit(0);
    }
    niters = atoll(argv[1]);

    /*创建线程并等待其终止*/
    pthread_create(&tid1, NULL, increase, (void*)niters);
    pthread_create(&tid2, NULL, decrease, (void*)niters);
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    if(cnt != 0)
        printf("Error! cnt=%d\n", cnt);
    else
        printf("Correct cnt=%d\n",cnt);

    sem_destroy(&mutex);        //销毁信号量
    exit(0);
}

void *increase(void *vargp)
{
    unsigned int i, niters = (unsigned int)vargp;
    
    for(i=0; i<niters; i++){
        sem_wait(&mutex);
        cnt++;
        sem_post(&mutex);
    }
    return NULL;
}

void *decrease(void *vargp)
{
    unsigned int i, niters = (unsigned int)vargp;

    for(i=0; i<niters; i++){
        sem_wait(&mutex);
        cnt--;
        sem_post(&mutex);
    }
    return NULL;
}
