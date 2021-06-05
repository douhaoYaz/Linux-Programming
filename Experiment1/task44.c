#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#define MAXSIZE 100

typedef struct _Word{
	char word[20];		//存放单词
	int count;			//计算出现频次
}Word;

typedef struct _WordTable{
	Word word[MAXSIZE];		//存放单词的单词表
	int count;			//单词表个数
}WordTable;

void Save2WordTable(WordTable *wordtable, char *temp)		//把单词保存到单词表
{
	int i;

	if(temp == NULL)										//要保存到单词表的字符串是空的
		return;
	if(wordtable->count == MAXSIZE){						//单词表已经满了的情况
		printf("单词表已满，保存失败\n");
		return;
	}

	for(i=0; i<wordtable->count; i++){						//单词表已存在要保存的单词的情况
		if(strcmp(wordtable->word[i].word, temp) == 0){
			wordtable->word[i].count++;
			return;
		}
		
	}

	//该单词首次出现的情况
	strcpy(wordtable->word[wordtable->count].word, temp);	//将单词temp保存到单词表
	wordtable->word[wordtable->count].count = 1;			//该单词个数设为1
	wordtable->count++;										//单词表存放的单词个数加1

}

void divide(char *temp)
{
	int i;
	
	if(temp == NULL)
		return;
	
	for(i=0; temp[i]!='\0'; i++){
		if(!( (temp[i]>='A' && temp[i]<='Z') || (temp[i]>='a' && temp[i]<='z') ))		//若temp中字符存在不是英文字符的其他字符
			temp[i] = ' ';															//将其替换为空格
	}

}

void sort(WordTable *wordtable)
{
	int i,j;
	Word temp;
	for(i=0; i<wordtable->count-1; i++){
		for(j=0; j<wordtable->count-1-j; j++){
			if(wordtable->word[j].count < wordtable->word[j+1].count){		//该单词的频次比下个单词的频次少，就把这两个单词互换
				temp = wordtable->word[j];
				wordtable->word[j] = wordtable->word[j+1];
				wordtable->word[j+1] = temp;
			}
		}
	}
}

int main()
{
	int i;
	FILE *fd;
	char temp[20], *part;
	WordTable wordtable;
	void Save2WordTable(WordTable *wordtable, char *temp);
	void divide(char *temp);
	void sort(WordTable *wordtable);

	wordtable.count=0;					//初始化单词表已存放单词个数为0
	fd = fopen("article.txt", "r");		//打开文件
	while(!feof(fd)){
		fscanf(fd, "%s", temp);
		divide(temp);
		part = strtok(temp, " ");
		do{
			Save2WordTable(&wordtable, part);		//将读到temp的单词保存到单词表中
		}while((part = strtok(NULL, " ")) != NULL);
	}

	for(i=0; i<wordtable.count; i++){			//输出单词表
		printf("%s:%d\n",wordtable.word[i].word, wordtable.word[i].count);
	}
	
	sort(&wordtable);							//按频度排序
	printf("出现频度最高的10个单词：\n");
	for(i=0; i<10; i++){			//输出按频度排序后单词表
		printf("%s:%d\n",wordtable.word[i].word, wordtable.word[i].count);
	}

	fclose(fd);		//关闭文件
}