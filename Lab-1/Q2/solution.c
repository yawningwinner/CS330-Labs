#include <stdio.h>
#include <stdlib.h>
#include "buffer_api.h"

int main(int argc, char* argv[]) {
	if(argc != 2){
		perror("usage: ./solution <testcase number>");
		return -1;
	}
	buffer_init(atoi(argv[1]));
	const struct ring_buffer *rb = buffer_get_base();
	u64 mhead = rb->data_head;
        u64 mtail = rb->data_tail;
	u64 sz = rb->data_size;
	//printf("tail : %ld, head:%ld\n", mtail,mhead);
	//mhead = mhead%sz;
	//mtail = mtail%sz;
	void* curr = rb->data_base + mtail%sz;
	u64 lost = 0;
	u64 uksize = 0;
	while(mtail<mhead){
		struct perf_event_header* temp = curr;
		u32 t = temp->type;
		if(t==0x1){
		  struct sample_event* pp = curr; 	
		  printf("0x%lx\n", pp->addr);
		}
		else if(t==0x2){
		 struct lost_event* pp = curr;		
		 lost += pp->lost;
		}
		else{
		 uksize+= temp->size;
		}
		curr =curr+ temp->size;
		if(curr==rb->data_base+sz)curr = rb->data_base;
		mtail +=temp->size;
	}
		
	/* ------ YOUR CODE ENDS HERE ------*/
	
	/* print formats */
	//printf("0x%...."); // to print sample addresses
	printf("number of lost records: %ld\n",lost);
	printf("unknown size: %ld\n",uksize);

	/* ------ YOUR CODE ENDS HERE ------*/
	buffer_exit();
	return 0;
}

