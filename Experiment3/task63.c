#include <pthread.h>
#include "wrapper.h"
#include <stdlib.h>
#include <malloc.h>

//! 选择使用普通的同步或是使用条件变量同步
#define PV

//* 选择使用普通的同步
#ifdef PV
#define N 20
sem_t avail, ready;
sem_t mutex;
int buf[N];
int inpos=0, outpos=0;
int sum_Pi=0, sum_Ci=0;     //生产者线程发送的数据和消费者线程接收的数据的求和，用于比较结果是否一致

void *Thread_Pi()
{
    //TODO 尝试用条件变量来写，进行共享变量的同步

    int i;
    int item;
    srand(time(0));     //用时间生成随机数种子
    for(i=0; i< 50; i++){
        item = rand()%100;  //随机生成0到99之间的整数
        sem_wait(&avail);
        sem_wait(&mutex);
        //临界区
        buf[inpos] = item;  //写入缓冲区
        inpos = (inpos + 1) % N;
        sum_Pi += item;
        //临界区结束
        sem_post(&mutex);
        sem_post(&ready);
    }
}

void *Thread_Ci()
{
    //TODO 尝试用条件变量来写，进行共享变量的同步

    int i;
    int item;

    for(i=0; i<50; i++){
        sem_wait(&ready);
        sem_wait(&mutex);
        //临界区
        item = buf[outpos];
        outpos = (outpos + 1) % N;
        sum_Ci += item;
        //临界区结束
        sem_post(&mutex);
        sem_post(&avail);
        printf("%d ", item);
    }
}

int main(int argc, char **argv)
{
    int k = atoi(argv[1]), m = atoi(argv[2]);   //从终端输入生产者线程数k，消费者线程数m
    int i;
    pthread_t tid[k+m];      //共k+m个线程
    sem_init(&avail, 0, N);
    sem_init(&ready, 0, 0);
    sem_init(&mutex, 0, 1);

    //*创建k个生产者线程
    for(i=0; i<k; i++)
        pthread_create(&tid[i], NULL, Thread_Pi, NULL);
    //*创建m个消费者线程
    for(i=k; i<k+m; i++)
        pthread_create(&tid[i], NULL, Thread_Ci, NULL);

    //*等待生产者线程结束
    for(i=0; i<k; i++)
        pthread_join(tid[i], NULL);
    //*等待消费者线程结束
    for(i=k; i<k+m; i++)
        pthread_join(tid[i], NULL);
    
    //!销毁信号量
    sem_destroy(&avail);
    sem_destroy(&ready);
    sem_destroy(&mutex);
    printf("\n生产者线程和消费者线程数据%s\n", sum_Pi == sum_Ci ? "正确" : "错误");
    exit(EXIT_SUCCESS);
}
#endif  // P/V


//* 选择使用条件变量同步
#ifdef CONDITION
#define N 20
//* 全局变量
int sum_Pi = 0, sum_Ci = 0;      //! 生产者线程发送的数据和消费者线程接收的数据的求和，用于比较结果是否一致

typedef struct {
    int *buf;                   /* 循环缓冲区队列数组 */
    int n;                      /* 缓冲区队列容量 */
    int outpos;                 /* 读出指针 */
    int inpos;                  /* 写入指针 */
    pthread_mutex_t lock;       /* 用于访问互斥区队列的互斥锁 */
    pthread_cond_t avail_cond;  /* 缓冲区队列有空闲单元的条件 */
    pthread_cond_t ready_cond;  /* 缓冲区队列有可用数据的条件 */
    int count;                  /* 缓冲区中可用数据单元数 */
}sbuf_t;

void sbuf_init(sbuf_t *sp)
{
    sp->buf = calloc(N, sizeof(int));
    sp->n = N;                  //设置缓冲区大小
    sp->outpos = sp->inpos = 0; //将读出指针和写入指针初始化为0
    pthread_mutex_init(&sp->lock, NULL);    //初始化互斥信号量为1
    pthread_cond_init(&sp->avail_cond, NULL);   //初始化条件变量
    pthread_cond_init(&sp->ready_cond, NULL);   //初始化条件变量
    sp->count = 0;              //初始化可用数据单元数
}

//* 清理缓冲区sp
void sbuf_deinit(sbuf_t *sp)
{
    pthread_mutex_destroy(&sp->lock);
    pthread_cond_destroy(&sp->avail_cond);
    pthread_cond_destroy(&sp->ready_cond);
    free(sp->buf);
}

//* 向共享缓冲区sp后插入数据item
void sbuf_insert(sbuf_t *sp, int item)
{
    //与上面使用普通同步方法不同的是，这里是先获得共享缓冲区队列的锁，再根据缓冲区队列空闲状态或有无数据的条件去读写
    //上面的方法是，先判断缓冲区队列空闲状态或有无数据的状态，然后再去获得锁
    pthread_mutex_lock(&sp->lock);
    if(sp->count == sp->n)      //缓冲区队列无空闲单元
        pthread_cond_wait(&sp->avail_cond, &sp->lock);
    
    sp->buf[sp->inpos] = item;      //向缓冲区存放数据
    sp->inpos = (sp->inpos + 1) % sp->n;
    sum_Pi += item;         //! 累加生产者线程产生的数据，用于与消费者线程接收的累加数据比较，判断程序正确性

    if(sp->count++ == 0)            //缓冲区队列中无可用数据
        pthread_cond_signal(&sp->ready_cond);
    pthread_mutex_unlock(&sp->lock);
}

//* 移除和返回共享缓冲区队列sp中的第一个数据项
int sbuf_remove(sbuf_t *sp)
{
    int item;
    pthread_mutex_lock(&sp->lock);
    if(sp->count == 0)          //缓冲区队列无可用数据
        pthread_cond_wait(&sp->ready_cond, &sp->lock);

    item = sp->buf[sp->outpos];     //从缓冲区读出数据
    sp->outpos = (sp->outpos + 1) % sp->n;
    sum_Ci += item;          //! 累加消费者线程产生的数据，用于与生产者线程接收的累加数据比较，判断程序正确性

    if(sp->count-- == sp->n)         //缓冲区队列无空闲单元
        pthread_cond_signal(&sp->avail_cond);
    pthread_mutex_unlock(&sp->lock);

    return item;
}

//* 生产者线程
void *Thread_Pi(void* vargp)
{
    sbuf_t *sp = (sbuf_t *)vargp;
    int item, i;
    srand(time(0));     //用时间生成随机数种子
    for(i=0; i<50; i++){
        item = rand()%100;  //随机生成0到99之间的正数
        sbuf_insert(sp, item);
    }
    
}

//* 消费者线程
void *Thread_Ci(void* vargp)
{
    sbuf_t *sp = (sbuf_t *)vargp;
    int item, i;
    for(i=0; i<50; i++){
        item = sbuf_remove(sp);
        printf("%d ", item);
    }
}

int main(int argc, char **argv)
{
    sbuf_t s;      //共享缓冲区
    int k = atoi(argv[1]), m = atoi(argv[2]);   //从终端输入生产者线程数k，消费者线程数m
    int i;
    pthread_t tid[k+m];      //共k+m个线程

    sbuf_init(&s); //初始化共享缓冲区

    //*创建k个生产者线程
    for(i=0; i<k; i++)
        pthread_create(&tid[i], NULL, Thread_Pi, (void*)&s);
    //*创建m个消费者线程
    for(i=k; i<k+m; i++)
        pthread_create(&tid[i], NULL, Thread_Ci, (void*)&s);


    // printf("here!\n");

    //*等待生产者线程结束
    for(i=0; i<k; i++)
        pthread_join(tid[i], NULL);
    //*等待消费者线程结束
    for(i=k; i<k+m; i++)
        pthread_join(tid[i], NULL);
    
    //! 销毁信号量
    sbuf_deinit(&s);
    printf("\n生产者线程和消费者线程数据%s\n", sum_Pi == sum_Ci ? "正确" : "错误");
    exit(EXIT_SUCCESS);
}

#endif  // CONDITION
