#include <stdio.h>
#include <string.h>
#define MAXSIZE 100

int main()
{
	FILE *fp1, *fp2;
	char buf[MAXSIZE];
	char *name, *num, *dept, *age, *sex;
	char buffer[100];

	//打开输入文件student.txt和输出文件csStudent
	if((fp1 = fopen("student.txt", "r")) == NULL){
		printf("Open Failed!");
		return 0;
	}

	if((fp2 = fopen("csStudent.txt", "a+")) == NULL){
		printf("Open Failed!");
		return 0;
	}

	//用fgets逐行读入，并用strtok来分割被:隔开的字符字串
	while(!feof(fp1))
	{
		fgets(buf, MAXSIZE, fp1);	//逐行读入文件student.txt的内容，并用字符数组buf存储每行内容
		if(strstr(buf, "计算机与网络安全学院")){	//如果该行字符串含有"计算机与网络安全学院的字符字串
			name = strtok(buf, ":");			//用strtok将:分隔的字符字串提取出来，strtok函数，第一次调用将buf字符数组首地址传进去，之后调用不用传地址，内部会有static函数保存指向地址
			num = strtok(NULL, ":");
			dept = strtok(NULL, ":");
			age = strtok(NULL, ":");
			sex = strtok(NULL, ":");
			sex[3] = 0;							//用0替换掉换行符'\n'，int类型的0会转换为char类型ascii码为0的空字符NULL，因为sex是char指针类型的
			sprintf(buffer,"%s:%s:%s:%s:%s\n", num, name, age, sex, dept);	//按要求格式输出到buffer
			fputs(buffer, fp2);					//将buffer里的内容输出到scStudent.txt里
		}
	}

	//关闭两个文件
	close(fp1);
	close(fp2);
	return 0;
}
