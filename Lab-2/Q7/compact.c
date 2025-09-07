#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

void compact(void *start, void *end)
{
  int cnt = 0;
  char* tracker=start, *invalid=start;
  while(tracker!=end){
	if(*tracker==0){tracker++;cnt--;}
	else{
		*invalid=*tracker;
		invalid++;
		tracker++;
	}
  }
 end =  sbrk(cnt);
  return;    
}
