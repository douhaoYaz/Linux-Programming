#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    int p11, p12, p121, p122;
    time_t t;

    p11 = fork();               //p1创建第一个子进程p11
    if(p11==0){                 //子进程p11执行这个分支
        t = time(&t);
        printf("当前时间是 %s", ctime(&t));
        exit(0);
    }

    if(p11>0){                  //父进程p1执行这个分支
        p12 = fork();           //p1创建第二个子进程p12
        if(p12==0){             //子进程p12执行这个分支
            p121 = fork();      //创建孙子进程p121
            if(p121==0){        //孙子进程p121执行这个分支
                printf("我的学号是201841510108\n");
                exit(0);
            }

            if(p121>0){         //子进程p12执行这个分支
                p122 = fork();  //创建孙子进程p122
                if(p122==0){    //孙子进程p122执行这个分支
                    printf("我的姓名是符浩扬\n");
                    exit(0);
                }

                printf("I am young brother process\n");
                exit(0);
            }
        }

        if(p12>0){              //父进程p1执行这个分支
            printf("I am father process\n");
            exit(0);
        }
    }

}