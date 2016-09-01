#include<stdio.h>
#include<sys/types.h>
#include<fcntl.h>
#include<string.h>
#include<stdlib.h>
#include<termios.h>
#include<unistd.h>
#include<sys/stat.h>
#define PAGELEN 24
#define LINELEN 512

unsigned long file_size;
unsigned long current_size;

void do_more(FILE*);
int see_more(FILE*,FILE*);
unsigned long get_file_size(const char*);

int main(int argc,char **argv)
{

	/*set terminal*/
	static struct termios oldt,newt;
	tcgetattr(STDIN_FILENO,&oldt);
	newt=oldt;
	newt.c_lflag&=~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO,TCSANOW,&newt);


	FILE *fp;
	if (argc==1)
		do_more(stdin);
	else
		while(--argc)
			if ((fp=fopen(*++argv,"r"))!=NULL)
			{
				file_size=get_file_size(*argv);
				current_size=0;
				do_more(fp);
				fclose(fp);
			}
			else
				exit(0);

	tcsetattr(STDIN_FILENO,TCSANOW,&oldt);
	return 0;
}

void do_more(FILE *fp) /*read PAGELEN lines ,then call see_more() for further instructions*/
{
	char line[LINELEN];
	int num_of_lines=0;
	int reply;
	FILE *fp_tty=fopen("/dev/tty","r");
	if (fp_tty==NULL) /*if open error*/
		exit(1);
	while(fgets(line,LINELEN,fp))
	{
		current_size+=(unsigned long)strlen(line);
		if (num_of_lines==PAGELEN)
		{
			reply=see_more(fp_tty,fp);
			if (reply==0)
				break;
			num_of_lines-=reply;
		}
		if (fputs(line,stdout)==EOF)
			exit(0);
		num_of_lines++;
	}
}
int see_more(FILE *cmd,FILE *fp)
{
	int c;
	if (fp==stdin)
	{
		printf("\033[7m --More--\033[m");
	}
	else
		printf("\033[7m --More--%.2f\% \033[m",((float)current_size/file_size)*100);
	while((c=getc(cmd))!=EOF)
	{
		printf("\033[20D");
		printf("\033[K");
		putchar('\n');
		if (c=='q')
			return 0;
		if (c==' ')  /*' '-> next page*/
			return PAGELEN; 
		if (c=='\n')
			return 1;
	}
	return 0;
}
unsigned long get_file_size(const char* path)
{
	unsigned long filesize=-1;
	struct stat statbuff;
	if (stat(path,&statbuff)>=0)
		filesize=statbuff.st_size;
	return filesize;
}
