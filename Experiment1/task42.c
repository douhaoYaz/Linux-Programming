#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

typedef struct _subject{
	char sno[20];
	char name[20];
	float chinese;
	float math;
	float english;
} subject;

int main()
{
	subject student[5];
	int fd,i;

	fd = open("data.txt", O_RDWR | O_CREAT | O_APPEND, 0777);	//打开文件，设置模式和读写权限
	
	for(i=0; i<5; i++){						//输出提示，从键盘读入5个学生的信息，并写入文件
		printf("Please enter no.%d student.number, student.name, chinese score, math score, English score:", i+1);
		scanf("%s%s%f%f%f", student[i].sno, student[i].name, &student[i].chinese, &student[i].math, &student[i].english);
		write(fd, &student[i], sizeof(student[i]));
	}

	lseek(fd, 0, SEEK_SET);
	read(fd, &student[0], sizeof(subject));
	lseek(fd, sizeof(subject), SEEK_CUR);
	read(fd, &student[1], sizeof(subject));
	lseek(fd, sizeof(subject), SEEK_CUR);
	read(fd, &student[2], sizeof(subject));
	
	for(i=0; i<3; i++)
		printf("%s%s%f%f%f\n",student[i].sno, student[i].name, student[i].chinese, student[i].math, student[i].english);
}
