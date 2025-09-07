#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<assert.h>
#include<string.h>
#include<sys/wait.h>
#define MAX_CHARS_IN_LINE 1000

int main(int argc, char **argv)
{
	char buf[MAX_CHARS_IN_LINE+1];
	char obuf[MAX_CHARS_IN_LINE+1];

	int fd[2];
	pipe(fd);
	int pid = fork();
	int f1,f2;
	f1 = open(argv[1],O_RDONLY);
	f2 = open(argv[2],O_WRONLY|O_CREAT,0666);
	if(!pid){
	  dup2(f2,1);
	  dup2(fd[0],0);
	  close(fd[1]);
	  execl("./encrypt","encrypt",NULL);	  
	}
	int f3 = open(argv[1],O_RDONLY);
	int sz = lseek(f3,0,SEEK_END);
	int cnt = 0;
	//write(fd[1],&sz,4);
	while(sz>0){
		int p = read(f1,&buf[cnt],1);
		if(buf[cnt]=='\n'){
			int pp=0;
			int tmp=cnt+1;
			while(tmp){
				pp++;
				tmp/=10;
			}
			char oye[cnt+2];
			for(int i=0;i<=cnt;i++){
			  oye[i] = buf[i];
			}
			oye[cnt+1]=0;
		//	snprintf(oye,cnt+1,"%s",buf);
			char newBuf[cnt+pp+2];
			sprintf(newBuf,"%d\n%s",cnt+1,oye);
		//	printf("%s\n",newBuf);
			write(fd[1],newBuf,cnt+pp+2);
			cnt = 0;
		}
		else{
		 cnt++;
		}
		sz-=p;
	}
	close(fd[1]);
	close(fd[0]);
	return 0;
}

