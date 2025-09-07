#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
int main(int argc, char **argv)
{
	/**
	 * TODO: Write your code here.
	 */
	int fd1,fd2;
	fd2 = open(argv[1],O_RDONLY);
	int count[6];
	for(int i =0;i<6;i++)count[i]=0;
	int q = lseek(fd2,0,SEEK_END);
	char* file_buf = (char*)malloc((q+6)*(sizeof(char)));
	char *tr = NULL;
	fd1 = open(argv[1], O_RDONLY);
	if(fd1< 0){
		printf("Error\n");
		exit(-1);
	}
	read(fd1, file_buf, q);
	int flag = 1;
	for(int i = 0;i<q-8;i++){
		char file_window[100];
		//if(flag){
		int j =0;
		for(j=0;j<8;j++){
			file_window[j] = file_buf[i+j];
			if(file_buf[i+j]=='(')break;
		}
		// if(i<10)printf("%s\n",file_window);

		file_window[j+1] = 0;
		if(strcmp(" openat(",file_window)==0)count[0]++;
		if(strncmp(" close(",file_window,7)==0)count[1]++;
		if(strncmp(" read(",file_window,6)==0)count[2]++;
		if(strncmp(" write(",file_window,7)==0)count[3]++;
		if(strncmp(" stat(",file_window,6)==0)count[4]++;	
		if(strcmp(" execve(",file_window)==0)count[5]++;
		//	i+=j;
		flag = 0;
		//	}
		if(file_buf[i]=='\n')flag = 1;
	}
	printf("openat: %d\n", count[0]);
	printf("close: %d\n", count[1]);
	printf("read: %d\n", count[2]);
	printf("write: %d\n", count[3]);
	printf("stat: %d\n", count[4]);
	printf("execve: %d\n", count[5]);

	exit(0);
	return 0;
}
