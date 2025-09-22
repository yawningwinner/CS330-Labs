#include<procinfo.h>
#include<lib.h>
#include<file.h>
#include<context.h>

static char* segment_names[MAX_MM_SEGS+1] = {"code", "rodata", "data", "stack", "invalid"}; 
static char* file_types[MAX_FILE_TYPE+1] = {"stdin", "stdout", "stderr", "reg", "pipe", "invalid"}; 

long get_process_info(struct exec_context *ctx, long cmd, char *ubuf, long len)
{
    long retval = -EINVAL;	
    /*
     * TODO your code goes in here
     * */
    switch (cmd) { 
	case GET_PINFO_GEN:
	    if(ubuf!=NULL && len>=80){
             /*
	     struct general_info *temp;
	     temp->pid = ctx->pid;
	     temp->ppid = ctx->ppid;
	     temp->pcb_addr = (u64)ctx;
	     memcpy(temp->pname,ctx->name,64);
	     memcpy(ubuf,temp,80);*/
             long addr = (u64)ctx;
             memcpy(ubuf,&ctx->pid,4);
             memcpy(ubuf+4,&ctx->ppid,4);
             memcpy(ubuf+8,&addr,8);
             memcpy(ubuf+16,&ctx->name,64);
	     retval = 1;  
	    }
	    break;
	case GET_PINFO_FILE:
	   int pp = 0;
	    for(int i = 0;i<32;i++){
	      if(ctx->files[i]!=NULL)pp++;
	    }  
	    if(ubuf!=NULL&&len>=20*pp){
		 pp = 0;
	    	for(int i =0 ;i<32;i++){
		    if(ctx->files[i]!=NULL){
		      strcpy(ubuf+pp*20,file_types[ctx->files[i]->type]);
		      memcpy(ubuf+pp*20+8,&(ctx->files[i]->mode),4);
		      memcpy(ubuf+pp*20+12,&(ctx->files[i]->ref_count),4);
		      memcpy(ubuf+pp*20+16,&(ctx->files[i]->offp),4);
			pp++;
		    }
		}
		retval = pp;
	    }
	    break;
	
	case GET_PINFO_MSEG: 
	    int p = 0;
	    for(int i = 0;i<MAX_MM_SEGS;i++){
	      if(&ctx->mms[i]!=NULL)p++;
	    } 
	    if(ubuf!=NULL&&len>=sizeof(struct mem_segment_info)*p){
		int  pp = 0;
	    	for(int i =0 ;i<MAX_MM_SEGS;i++){
		    if(&ctx->mms[i]!=NULL){
			   struct mem_segment_info *info = (struct mem_segment_info *) (ubuf + pp*sizeof(struct mem_segment_info));
			   strcpy(info->segname,segment_names[i]);
			   info->start =ctx->mms[i].start;  
			   info->end =ctx->mms[i].end;  
			   info->next_free =ctx->mms[i].next_free;  
			   if(ctx->mms[i].access_flags &1)info->perm[0] = 'R';
			   else info->perm[0] = '_';
			   if(ctx->mms[i].access_flags &2)info->perm[1] = 'W';
			   else info->perm[1] = '_';
			   if(ctx->mms[i].access_flags &4)info->perm[2] = 'X';
			   else info->perm[2] = '_';
			   info->perm[3] = '\0';
			   pp++;
		    }
		}
		retval = pp;
	    }

		break;
	case GET_PINFO_VMA: 
	     p = 0;
	    struct vm_area *temp = ctx->vm_area;
	      while(temp!=NULL){
	         p++;
		 temp = temp->vm_next;
	      }
	      p--;
	    if(ubuf!=NULL&&len>=sizeof(struct vm_area_info)*p){
		// printk("aa gya bhai\n");
		int  pp = 0;
		temp = ctx->vm_area->vm_next;
		    while(temp!=NULL){
			   struct vm_area_info *info = (struct vm_area_info*) (ubuf + pp*sizeof(struct vm_area_info));
			   info->start =temp->vm_start;  
			   info->end =temp->vm_end;  
			   if(temp->access_flags &1)info->perm[0] = 'R';
			   else info->perm[0] = '_';
			   if(temp->access_flags &2)info->perm[1] = 'W';
			   else info->perm[1] = '_';
			   if(temp->access_flags &4)info->perm[2] = 'X';
			   else info->perm[2] = '_';
			   info->perm[3] = '\0';
			   pp++;
			   temp = temp->vm_next;
		    }
		retval = pp;
	    }
    }
    return retval;    
}
