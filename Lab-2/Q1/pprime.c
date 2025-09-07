#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<math.h>
#include<sys/time.h>
#include<sys/wait.h>


extern int isPrime(long x); // Returns zero if composite.
                            // Returns one if prime. You may use this if you want. 
int find_primes(long M)
{
  //Your code goes in here
 int status1=0,status2=0,ans=0;
 int pid = fork();
	if(pid<0){
	 exit(-1);
	} 
	if(!pid){
	  for(long i =2;i<=M/2;i++){
	  	if(isPrime(i))status1++;
		status1%=256;
	  }
  	 exit(status1);
   }
  // long x = waitpid(pid,&ans1,WEXITSTATUS(ans1));
   int pid1 = fork();
   if(!pid1){
	   
   	for(long i = M/2+1;i<=M;i++){
		if(isPrime(i))status2++;	
		status2%=256;
	}
	exit(status2);
   }
   wait(&ans);
   int x = WEXITSTATUS(ans);
   wait(&ans);
   x+=WEXITSTATUS(ans);
   return x%256;  
}
