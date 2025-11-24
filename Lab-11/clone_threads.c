#include<clone_threads.h>
#include<entry.h>
#include<context.h>
#include<memory.h>
#include<lib.h>
#include<mmap.h>
#include<fork.h>
#include<page.h>

extern int destroy_user_mappings(struct exec_context *ctx);

static void set_kstack_of_thread(struct exec_context *ctx)
{
   ctx->os_stack_pfn = os_pfn_alloc(OS_PT_REG);
   ctx->os_rsp = (((u64) ctx->os_stack_pfn) << PAGE_SHIFT) + PAGE_SIZE;
   stats->num_processes++;
   ctx->type = EXEC_CTX_USER_TH;	
}

//XXX Do not modify anything above this line

/*
  system call handler for clone, create thread like execution contexts
*/
long do_clone(void *th_func, void *user_stack, void *user_arg) 
{
//	return pid;
  int ctr;
  struct exec_context *new_ctx = get_new_ctx();  //This is to be used for the newly created thread
  struct exec_context *ctx = get_current_ctx();
  u32 pid = new_ctx->pid;
 //return pid;
  struct thread *n_thread;

  if(!ctx->ctx_threads){  // This is the first thread
          ctx->ctx_threads = os_alloc(sizeof(struct ctx_thread_info));
          bzero((char *)ctx->ctx_threads, sizeof(struct ctx_thread_info));
          ctx->ctx_threads->pid = ctx->pid;
  }

 /* XXX Do not modify anything above. Your implementation goes here */

 // TODO your code goes here
  
   int cnt=0;
   while(ctx->ctx_threads->threads[cnt].pid!=0)cnt++;
   struct thread* th = &(ctx->ctx_threads->threads[cnt]);
   th->pid=pid;
  // printk("dekh bhai :%d  %d\n",cnt,th->pid);
   //printk("ye bhi : %d\n", ctx->ctx_threads->threads[0].pid);
   th->parent_ctx=ctx;
   th->status = TH_USED;
   *new_ctx=*ctx;
   new_ctx->pid=pid;
   new_ctx->ppid=ctx->pid;
   new_ctx->type=EXEC_CTX_USER_TH;
   new_ctx->ctx_threads=NULL;
   new_ctx->regs.entry_rsp = (u64)user_stack;
   new_ctx->regs.entry_rip = (u64)th_func;
   new_ctx->regs.rbp = (u64)user_stack;
   new_ctx->regs.rdi = ((u64)user_arg);
   new_ctx->state = READY;
   new_ctx->lock = ctx->lock;
  // printk("new_ctx : %x, ctx : %x \n", &new_ctx->lock[0].sem.wait_queue,&ctx->lock[0].sem.wait_queue);
 //End of your logic
  
 //XXX The following two lines should be there. 
  
  set_kstack_of_thread(new_ctx);  //Allocate kstack for the thread
  return pid;
}



//handler for exit()
void do_exit(u8 normal)
{
   struct exec_context *ctx = get_current_ctx();
   struct exec_context *new_ctx ;
   //printk("kaun bula rha : %d\n",ctx->pid);
   if(ctx->type==EXEC_CTX_USER_TH){
	handle_thread_exit(ctx,normal);

  release_context(ctx); 
  new_ctx = pick_next_context(ctx);
  dprintk("Scheduling %s:%d [ptr = %x]\n", new_ctx->name, new_ctx->pid, new_ctx); 
  schedule(new_ctx);  //Calling from exit
   }else{
	   cleanup_all_threads(ctx);
	   do_exit2(normal);

   }
   return;
}

// XXX Reference implementation for a process exit
// You can refer this to implement your version of do_exit

void do_exit2(u8 normal) 
{
  int ctr;
  struct exec_context *ctx = get_current_ctx();
  struct exec_context *new_ctx;

 
  do_file_exit(ctx);   // Cleanup the files

  // cleanup of this process
  destroy_user_mappings(ctx); 
  do_vma_exit(ctx);
  if(!put_pfn(ctx->pgd)) 
      os_pfn_free(OS_PT_REG, ctx->pgd);   //XXX Now its fine as it is a single core system
  if(!put_pfn(ctx->os_stack_pfn))
     os_pfn_free(OS_PT_REG, ctx->os_stack_pfn);
  release_context(ctx); 
  new_ctx = pick_next_context(ctx);
  dprintk("Scheduling %s:%d [ptr = %x]\n", new_ctx->name, new_ctx->pid, new_ctx); 
  schedule(new_ctx);  //Calling from exit

}



////////////////////////////////////////////////////////// Semaphore implementation ////////////////////////////////////////////////////
//
//


// A spin lock implementation using cmpxchg
// XXX you can use it for implementing the semaphore
// Do not modify this code

static void spin_init(struct spinlock *spinlock)
{
	spinlock->value = 0;
	//printk("spinlock initialised\n");
}

static void spin_lock(struct spinlock *spinlock)
{
	unsigned long *addr = &(spinlock->value);

	asm volatile(
		"mov $1,  %%rcx;"
		"mov %0,  %%rdi;"
		"try: xor %%rax, %%rax;"
		"lock cmpxchg %%rcx, (%%rdi);"
		"jnz try;"
		:
		: "r"(addr)
		: "rcx", "rdi", "rax", "memory"
	);
}

static void spin_unlock(struct spinlock *spinlock)
{
	spinlock->value = 0;
}

static int init_sem_metadata_in_context(struct exec_context *ctx)
{
   if(ctx->lock){
	   printk("Already initialized MD. Call only for the first time\n");
	   return -1;
   }
   ctx->lock = (struct lock*) os_alloc(sizeof(struct lock) * MAX_LOCKS);
   if(ctx->lock == NULL){
			printk("[pid: %d]BUG: Out of memory!\n", ctx->pid);
                        return -1;
   }
	
   for(int i=0; i<MAX_LOCKS; i++)
			ctx->lock[i].state = LOCK_UNUSED;
}

// XXX Do not modify anything above this line

/*
  system call handler for semaphore creation
*/
int do_sem_init(struct exec_context *current, sem_t *sem_id, int value)
{
	if(current->lock == NULL)
		init_sem_metadata_in_context(current);
        // TODO Your implementation goes here
	struct exec_context* ctx = current;
	int cnt=8;
	for (int i = 0; i<8; i++)
	{
		if(ctx->lock[i].state == LOCK_UNUSED)
		{
			cnt = i;
			break;
		}
	}
	if (cnt == 8) return -EAGAIN;
	ctx->lock[cnt].state=LOCK_USED;
	*(sem_id) = cnt;
	ctx->lock[cnt].id = *(sem_id);
	ctx->lock[cnt].sem.value = value;
	ctx->lock[cnt].sem.wait_queue = NULL;
	spin_init(&(ctx -> lock[cnt].sem.lock));	
	return 0;
}

/*
  system call handler for semaphore acquire
*/

int do_sem_wait(struct exec_context *current, sem_t *sem_id)
{
		
	struct exec_context* ctx = current;
	int cnt=8;
	for (int i = 0; i<8; i++)
	{
		if(ctx->lock[i].state == LOCK_USED && ctx->lock[i].id == *sem_id)
		{
			cnt = i;
			break;
		}
	}

	if (cnt == 8) return -EAGAIN;
	spin_lock(&(ctx->lock[cnt].sem.lock));
	if (ctx -> lock[cnt].sem.value >0 )
	{	
		ctx->lock[cnt].sem.value --;
		spin_unlock(&(ctx->lock[cnt].sem.lock));
	}
	else 
	{
		struct exec_context* temp = ctx->lock[cnt].sem.wait_queue;
		if(temp == NULL)
		{
			ctx->lock[cnt].sem.wait_queue = current;
		}
		else
		{
			while(temp -> next != NULL)
			{
				temp = temp -> next;
			}
			temp -> next = current;		
		}
		current->next=NULL;
		current->state = WAITING;
	//	printk("%x wait\n",ctx->lock[cnt].sem.wait_queue);
	//	printk("Wait mein daal diya %d ko \n",current->pid);
		spin_unlock(&(ctx->lock[cnt].sem.lock));
		struct exec_context* new_ctx = pick_next_context(ctx);
  		schedule(new_ctx);  //Calling from exit
	}
	return 0;
}

/*
  system call handler for semaphore release
*/
int do_sem_post(struct exec_context *current, sem_t *sem_id)
{
	struct exec_context* ctx =  current;
	int cnt=8;
	for (int i = 0; i<8; i++)
	{
		if(ctx->lock[i].state == LOCK_USED && ctx->lock[i].id == *sem_id)
		{
			cnt = i;
			break;
		}
	}

	if (cnt == 8) return -EAGAIN;
	spin_lock(&(ctx->lock[cnt].sem.lock));
	if (ctx -> lock[cnt].sem.wait_queue == NULL) ctx->lock[cnt].sem.value ++;
	else 
	{
		struct exec_context* temp =ctx->lock[cnt].sem.wait_queue;
		ctx -> lock[cnt].sem.wait_queue = temp -> next;
		temp -> next = NULL;
		temp -> state = READY;
	//	printk("Wait se nikal diya %d ko\n",temp->pid);
	}
	
	spin_unlock(&(ctx->lock[cnt].sem.lock));
	return 0;
}
