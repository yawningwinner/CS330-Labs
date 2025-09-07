#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main (int argc, char **argv) {

	/**
	 * TODO: Write your code here.
	 */
	int fd1,fd2;
	fd2 = open(argv[2],O_RDONLY);
	int n = strlen(argv[1]);
	int q = lseek(fd2,0,SEEK_END);
	char buf[q+1];
	char *tr = NULL; 
	fd1 = open(argv[2], O_RDONLY);
	if(fd1< 0){
		printf("Error\n");
		exit(-1);
	}
	read(fd1, buf, q);
	for(int i = 0;i<=q;i++){
		char buff[n];
		strncpy(buff,&buf[i],n);
		buff[i+n] = 0;
		//   printf("%s\n",buff);
		if(strcmp(buff,argv[1])==0){
			printf("FOUND\n");
			exit(1);
		}
	}
	printf("NOT FOUND\n");
	//printf("fd1 position = %ld, fd2 position = %ld\n", lseek(fd1, 0, SEEK_CUR), lseek(fd2, 0, SEEK_CUR));  
	//assert(read(fd2, buf, 32) == 32);
	// printf("fd1 position = %ld, fd2 position = %ld\n", lseek(fd1, 0, SEEK_CUR), lseek(fd2, 0, SEEK_CUR));
	return 0;
}
