#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<assert.h>
#include<sys/mman.h>
#include<math.h>
#define ull unsigned long long
long how_large(void *address)
{
	/*
	 *  TODO: Your code
	 */ 
	unsigned long long mr = (unsigned long long) address;
	mr/=4096;
	mr*=4096;
	void* temp = (void*)mr;
	void* ptr = mmap(temp,1,PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED_NOREPLACE,0,0);
	if(ptr==temp){
		munmap(ptr,1);
		return 0LL;
	}
	ull left = mr;
	ull right = left+4096;
	left -=4096;
	while(left>0){
		void* pp = (void*)left;
		void* pr = mmap(pp,1,PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED_NOREPLACE,0,0);

		//void* pr = mmap(pp,1,PROT_NONE,MAP_FIXED_NOREPLACE,0,0);
		if(pr==pp){
			munmap(pr,1);
			break;
		}
	//	printf("%p\n",left);
		left-=4096;
	} 
	while(1){
		void* pp = (void*)right;
		void* pr = mmap(pp,1,PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED_NOREPLACE,0,0);
		if(pr==pp){
			munmap(pr,1);
			break;
		}
		right+=4096;
	} 
     	
	return right-left-4096;
	return -1;    
}
