#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<math.h>
#include<sys/time.h>
#include<sys/wait.h>

int main(int argc, char **argv)
{
   /*Your code goes here */
  if(argc==2){
	if(atol(argv[1])>2){
                long p = atol(argv[1]);
                sprintf(argv[1],"%ld",p-1);
		char pr[32];
                sprintf(pr,"%ld",p);
                if(execl("./fact", "fact",argv[1],pr, NULL))
                         perror("execp");
        }
        else{
         long ans = atol(argv[1]);
         printf("%ld\n",ans);
         exit(1);
        }
	
  	if(execl("./fact", "fact",argv[1],"1", NULL));
        	perror("exec");
  } 
  else{
  	if(atol(argv[1])>2){
		long p = atol(argv[1]);
		long ans = atol(argv[2]);
		ans = ans*p;
		sprintf(argv[1],"%ld",p-1);
		sprintf(argv[2],"%ld",ans);
		if(execl("./fact", "fact",argv[1],argv[2], NULL))
       			 perror("execp");
	}
	else{
	 long ans = atol(argv[2])*atol(argv[1]);
	 printf("%ld\n",ans);
	 exit(1);
	}
  }
   return 0;
}
