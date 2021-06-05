#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include "wrapper.h"

void *task1(void *vargp)
{
    int i;
    for(i=0; i<5; i++){
        printf("My name is douhao\n");
        sleep(1);
    }
    return NULL;
}

void *task2(void *vargp)
{
    int i;
    for(i=0; i<5; i++){
        printf("My student number is 201841510108\n");
        sleep(1);
    }
    return NULL;
}

void *task3(void *vargp)
{
    int i;
    time_t t;
    char *ch;
    
    time(&t);
    ch = ctime(&t);
    //char *ch = ctime(&time(&t));
    for(i=0; i<5; i++){
        printf("Current time %s", ch);
        sleep(1);
    }
    return NULL;
}

int main()
{
    pthread_t t1, t2, t3;
    void *ret;
    
    pthread_create(&t1, NULL, task1, NULL); //创建对等线程
    pthread_create(&t2, NULL, task2, NULL);
    pthread_create(&t3, NULL, task3, NULL);
    pthread_exit(ret);     //主线程等待所有对等线程终止，然后终止主线程
}
