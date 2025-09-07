#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<math.h>
#include<sys/time.h>
#include<sys/wait.h>
#include<string.h>
#include<sys/syscall.h>
extern int isPrime(long x); // Returns zero if composite.
                            // Returns one if prime. 

long find_primes(int num_cp, long M)
{
   /*
    *  TODO 
    *  Your code goes here
    */
   int fd[num_cp][2];
   int p = M/num_cp;
   for(int i= 0;i<num_cp;i++){
   	syscall(SYS_pipe,fd[i]);
   	int pid = fork();
	if(!pid){
	  int cnt = 0;
	  for(int j = i*p;j<(i+1)*p;j++){
		if(j==M+1)break;
	  	if(isPrime(j))cnt++;
	  }
	  if(i==num_cp-1){
	    for(int j = (i+1)*p;j<=M;j++){
	       if(isPrime(j))cnt++;
	    }
	  }
  	 write(fd[i][1],&cnt,4);
	 exit(i+1);	 
	}
   }
   int x = 0;
   long ans = 0;
   int cc = 0;
   for(int i= 0;i<num_cp;i++){
   	wait(&x);
	long p = 0;
	cc = WEXITSTATUS(x);
	read(fd[cc-1][0],&p,4);
	ans+=p;
   }
   return ans;	
   return 0;   
}
