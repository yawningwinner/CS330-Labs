#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>

// Use Library function such as fopen, geline, fread etc
// use manpages as mentioned in the writeup. 
// -----------------------
//
//
// Your solution begins here
int main(int argc, char *argv[]) {
	FILE *f = fopen(argv[2],"r");
	if (f == NULL){
		perror("fopen");
               exit(EXIT_FAILURE);
		
	}
	int l=0,w=0,c=0;
	char* line= NULL;
	size_t capacity = 0;
	size_t nread;
	while((nread = getline(&line,&capacity,f))!=-1){
		l++;
		int flag = 1;
		for(size_t i = 0;i<nread;i++){
			if(!flag && isspace(line[i])){
				w++;
				//printf("dekh %ld\n",c+i);
			}
			flag = isspace(line[i]);
		}
		c = c + nread;
	}
	free(line);
	if(strcmp("-l",argv[1])==0){
		printf("%d\n",l);		
	}
       else if(strcmp("-w",argv[1])==0){
                printf("%d\n",w);
        }
       else if(strcmp("-c",argv[1])==0){
                printf("%d\n",c);
        }
	else return 1;

	return 0;
}

