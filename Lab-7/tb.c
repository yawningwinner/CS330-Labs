#include<tb.h>
#include<lib.h>
#include<file.h>
#include<context.h>
#include<memory.h>

/*
 * *  Trace buffer implementation.
 *
 */


///////////////////////////////////////////////////////////////////////////
////           TODO:     Trace buffer functionality                   /////
///////////////////////////////////////////////////////////////////////////


// Check whether passed buffer is valid memory location for read.
static int tb_validate(unsigned long buff, u32 count, int acflags) 
{
    //  printk("Error validate\n");	
      	int i = -1;
	struct exec_context *ctx = get_current_ctx();
	for(int j = MM_SEG_CODE;j<MAX_MM_SEGS;j++) {
		if(j==MM_SEG_STACK) {
		if(ctx->mms[j].start <= buff && ctx->mms[j].end>buff+count){
			i = j;
			break;
		}
			
		}
		if(ctx->mms[j].start <= buff && ctx->mms[j].next_free>buff+count){
			i = j;
			break;
		}
	}
	if(i==-1) {
	   struct vm_area* temp = ctx->vm_area;
   	   while(temp) {
	   	if(temp->vm_start<=buff && temp->vm_end>buff+count)break;
			temp = temp->vm_next;
	   }
	   if(temp==NULL)return -1;
	   //printk("vm area %d, flag : %d\n",acflags,temp->access_flags);
	   if(acflags==0 && (temp->access_flags&1)==1)return 1;   
	   if(acflags==1 && (temp->access_flags&2)==2)return 1;
	   return -1;   
	  // if(acflags==0 && temp->access_flags&1==1)return 1;   
	}
	else {
	  if(acflags==0)return 1;
	  if(i==MM_SEG_CODE || i==MM_SEG_RODATA) {
	  	return -1;
	  }
	  return 1;
	}
	return -1;
}

static long tb_close(struct file *filep)
{	
	if(filep==NULL)return -EINVAL;
	os_page_free(USER_REG, filep->tb->buff);
	os_free(filep->tb, sizeof(struct tb_info));
	os_free(filep->fops, sizeof(struct fileops));
	os_free(filep, sizeof(struct file));
        //printk("Error\n");	
        return 0;
}

static int tb_read(struct file *filep, char *buff, u32 count)
{
    if(tb_validate((u64)buff,count,1)==-1)return -EBADMEM;
    //if(tb_validate((u64)buff,count,1)==-EBADMEM)return -EBADMEM;
    u32 cnt = filep->tb->w-filep->tb->r;
    if(filep==NULL || buff==NULL || filep->mode==O_WRITE)return -EINVAL;
    /* if(cnt<0)cnt+=4096;
    if(cnt==0 && filep->tb->w!=0) {
    	cnt+=4096;
    } */
    int c = 0;
    int r = filep->tb->r;
    while(count>0 && cnt>0){
	//r = r%4096;
    	buff[c++] = filep->tb->buff[r%4096];
	cnt--;
	r++;
	count--;
    }
    
    filep->tb->r = r;
    //printk("Error\n");	
    return c;

}

static int tb_write(struct file *filep, char *buff, u32 count)
{
    if(tb_validate((u64)buff,count,0)==-1)return -EBADMEM;
    //if(tb_validate((u64)buff,count,0)==-EBADMEM)return -EBADMEM;
    u32 cnt = 4096-filep->tb->w+filep->tb->r;
    if(filep==NULL || buff==NULL || filep->mode==O_READ)return -EINVAL;
    int c = 0;
    int w = filep->tb->w;
    while(count>0 && cnt>0){
//	w = w%4096;
    	filep->tb->buff[w%4096] = buff[c++];
	cnt--;
	w++;
	count--;
    }
    filep->tb->w = w; 
    //printk("Error\n");	
    return c;
}

int sys_create_tb(struct exec_context *current, int mode)
{
    int ret_fd = -1;
    for(int i = 0;i<MAX_OPEN_FILES;i++) {
    	if(current->files[i]==NULL) {
	    ret_fd = i;
	    break;
	}
    }
    if(ret_fd==-1)return -EINVAL;
    struct file *f = (struct file*)os_alloc(sizeof(struct file));
    f->type = TRACE_BUFFER;
    f->mode = mode;
    f->offp = 0;
    f->ref_count = 1;
    f->inode = NULL;
    f->pipe = NULL;
    f->fops = (struct fileops*)os_alloc(sizeof(struct fileops));
    f->fops->read = &tb_read; 
    f->fops->write = &tb_write; 
    f->fops->close = &tb_close;
    struct tb_info* info = (struct tb_info*)os_alloc(sizeof(struct tb_info));
    info->buff = (char*)os_page_alloc(USER_REG);
    f->tb = info; 
    current->files[ret_fd] = f;
    //printk("Error\n");
	    
    return ret_fd;
}
