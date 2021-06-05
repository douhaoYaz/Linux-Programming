#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

//#define debug
#define MAXLINE 100
#define MAXARGS 10

int main()
{
    void execute(char *cmdline);    //execute函数声明
    char cmdline[MAXLINE];      //命令行缓冲区

    while(1){
        printf("%%");

        fgets(cmdline, MAXLINE, stdin);     //读取命令行
        if(feof(stdin))
            exit(0);

        execute(cmdline);

    }
}

void execute(char *cmdline)
{
    int builtin_command(char **argv);       //buildin_command函数声明
    int parseline(char *buf, char **argv, int *redirect, int *my_pipe);  //parseline函数声明
    void redirect(char **argv);             //redirect函数声明
    void myPipe(char **argv);               //myPipe函数声明
    char *argv[MAXARGS];        //execve函数的参数表
    char buf[MAXLINE];          //保存修改后的命令行
    int bg;                     //是否在后台运行
    pid_t pid;                  //子进程PID
    int redirection;            //重定向标志
    int my_pipe;                //管道标志

    strcpy(buf, cmdline);
    bg = parseline(buf, argv, &redirection, &my_pipe);      //解析命令行
    if(argv[0] == NULL)
        return;                 //如果第一个参数为空，则忽略命令

    if(!builtin_command(argv)){
        if(my_pipe == 1){        //如果命令使用了管道
            printf("running myPipe\n");
            myPipe(argv);       //执行myPipe函数
            printf("After running myPipe\n");
        }
        else if((pid = fork()) == 0){    //创建子进程
            if(redirection == 1){   //如果命令使用了重定向   
                redirect(argv);
                exit(0);
            }
            else if(execvp(argv[0], argv) < 0){
                printf("%s:Command not found.\n",argv[0]);
                exit(0);
            }
        }

#ifdef debug
            printf("see if will come here\n");
#endif //debug

        if(!bg){                //前台执行
            int status;
            if(waitpid(-1, &status, 0) < 0)
                perror("waitpid error");
#ifdef debug
            printf("father process wait over\n");
#endif //debug            
        }
        else
            printf("%d %s", pid, cmdline);
    }
    return;
}

int builtin_command(char **argv)
{
    if( (!strcmp(argv[0],"exit")) || (!strcmp(argv[0], "logout")))        //内置命令exit或logout
        exit(0);
    if(!strcmp(argv[0],"&"))           //忽略由&开始的命令串
        return 1;
    return 0;                          //非内置命令
}

int parseline(char *buf, char **argv, int *redirection, int *my_pipe)
{
    char *delim;            //指向第1个分隔符
    int argc;               //字符串数组args中命令行参数的个数
    int bg;                 //后台作业

    buf[strlen(buf)-1] = ' ';    //用空格替换行末换行符
    while(*buf && (*buf==' '))   //删除行首空格
        buf++;
    
    //创建argv数组
    argc = 0;
    *redirection = 0;           //默认指令不是重定向
    while((delim = strchr(buf,' '))){
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while(*buf && (*buf == ' '))
            buf++;
        
        //判断命令行参数中是否存在重定向命令
        if( (!strcmp(argv[argc-1], "<")) || (!strcmp(argv[argc-1], ">")) || (!strcmp(argv[argc-1], ">>")))
            *redirection = 1;
        if(!strcmp(argv[argc-1], "|")){
            *my_pipe = 1;
            printf("Detected pipe\n");
        }
    }
    argv[argc] = NULL;

    if(argc == 0)           //忽略空行
        return 1;
    
    //命令是否应在后台执行
    if((bg = (*argv[argc-1] == '&'))!= 0)
        argv[--argc] = NULL;
    return bg;
}

void redirect(char **argv)          //重定向
{
    int fd;

    if(!strcmp(argv[1], "<")){      //若是输入重定向 <
        fd = open(argv[2], O_RDONLY, 0);
        close(0);                   //关闭标准输入
        dup(fd);                    //将fd中的文件指针赋值到描述符0
    }
    else{                           //若是输出重定向 > 或 >>
        fd = open(argv[2], O_WRONLY, 0);
        close(1);                   //关闭标准输出

        if(!strcmp(argv[1], ">"))   //若是>，即覆盖文件
            lseek(fd, 0, SEEK_SET);
        else if(!strcmp(argv[1], ">>")) //若是>>，即追加文件
            lseek(fd, 0, SEEK_END);
        
        dup(fd);                    //将fd中的文件指针重定向到描述符1
    }

    execlp(argv[0],argv[0],argv[2],NULL);
}


/**
* * For Highlighted Text
* ! For Errors and Warnings
* ? For Queries and Question
* //For Strike through
* TODO For TODOs 
*/
void myPipe(char **argv)
{
    int pid1, pid2, fds[2];
    pipe(fds);

    if((pid1 = fork()) == 0){       //创建子进程pid1，读入
        close(0);                   //关闭标准输入
        dup(fds[0]);                //将子进程的读管道文件描述符fds[0]复制到文件描述符0，文件描述符0的内容就是指向子进程读管道这个文件对象的指针，因为命令默认是从文件描述符0指向的文件对象读数据
        close(fds[0]);              //将原先内容为指向子进程读管道的指针的文件描述符fds[0]关闭，因为已经将文件描述符0的内容指向子进程读管道了，这个fds[0]已经没用了
        close(fds[1]);              //将原先内容为指向子进程写管道的指针的文件描述符fds[1]关闭
        //TODO 加载程序、exit
        printf("see when running execlp of pid1\n");
        execlp(argv[2], argv[2], NULL);     //例如管道命令是" ls | sort "，这个子进程执行的是sort，因此用argv[2]
        printf("see if running after execlp of pid1\n");
        exit(EXIT_FAILURE);
    }
    else{                           //父进程执行这个分支
        if((pid2 = fork()) == 0){   //创建子进程pid2，写入
            close(fds[0]);           //关闭pid2子进程的读管道，因为它不需要读
            close(1);               //关闭标准写入
            dup(fds[1]);            //将子进程的写管道文件描述符fds[1]复制到文件描述符1，因为命令执行结果是默认输出到文件描述符1中的
            close(fds[1]);          //子进程的写管道文件描述符fds[1]复制到文件描述符1之后，fds[1]也没用了
            printf("see when running execlp of pid2\n");
            execlp(argv[0], argv[0], NULL);     //例如管道命令是" ls | sort "，这个子进程执行的是ls，因此用argv[0]
            printf("see if running after execlp of pid2\n");
            exit(EXIT_FAILURE);
        }
        else{                       //父进程执行这个分支
            printf("Father not to exit\n");
            //exit(0);                //父进程结束，因为实现管道命令用两个子进程就可以了
        }
    }
}

