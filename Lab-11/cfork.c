#include <fork.h>
#include <page.h>
#include <mmap.h>
#include <apic.h>

/* #################################################*/

static inline void invlpg(unsigned long addr) {
    asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}
/**
 * cfork system call implemenations
 */
void page_walk (u64 par_pgd, u64 child_pgd, u64 vaddr, u64 flags)
{
	u64 mask9 = (1LL<<9) - 1;
	u64 mask12 = (1LL<<12)-1;
	u64 temp = vaddr;
	u64 L4off = (temp >>12)&mask9;
	temp >>= 12;
	u64 L3off = (temp>>9)&mask9;
	temp>>=9;
	u64 L2off = (temp>>9)&mask9;
	temp>>=9;
	u64 L1off = (temp>>9)&mask9;
	
	u64* L1pte = (u64 *)osmap(par_pgd) + L1off;
	u64* L1child = (u64 *)osmap(child_pgd) + L1off;
	if(*L1child == 0x0 )
	{
		u64 pfn = os_pfn_alloc(OS_PT_REG);
		*L1child = (*L1child) | (pfn<<12);
		*L1child |= ((*L1pte)&mask12);	
	}
	if (((*L1pte) & 1) == 0)
	{
		return;
	}
	u64* L2pte = (u64 *)osmap((*L1pte)>>12) + L2off;
	u64* L2child = (u64 *)osmap((*L1child)>>12) + L2off;
	if(*L2child == 0x0 )
	{
		u64 pfn = os_pfn_alloc(OS_PT_REG);
		*L2child = (*L2child) | (pfn<<12);
		*L2child |= ((*L2pte)&mask12);	
	}
	
	if (((*L2pte) & 1) == 0)
	{
		return;
	}
	u64* L3pte = (u64 *)osmap((*L2pte)>>12) + L3off;
	u64* L3child = (u64 *)osmap((*L2child)>>12) + L3off;
	if(*L3child == 0x0 )
	{
		u64 pfn = os_pfn_alloc(OS_PT_REG);
		*L3child = (*L3child) | (pfn<<12);
		*L3child |= ((*L3pte)&mask12);	
	}

	if (((*L3pte) & 1) == 0)
	{
		return;
	}
//	printk("L3 tak ho gya\n");
	u64* L4_pte = (u64 *)osmap((*L3pte)>>12) + L4off;
	u64* L4child = (u64 *)osmap((*L3child)>>12) + L4off;
	if (((*L4_pte) &1) == 0) return;
	if(*L4child == 0x0 )
	{
	//	printk("pfn:%x pte:%x\n",(*L4_pte)>>12,*L4_pte);	
		s8 pp = get_pfn((*L4_pte)>>12);
		*L4child = (*L4_pte);
		if (*L4child & 8 != 0)
		{
			*L4child ^=8;
			*L4_pte ^=8;
		       invlpg(vaddr);	
		}

	}
	//printk("L4 bhi ho gya, aur pfn : %d\n", get_pfn((*L4_pte)>>12));
	

}
long do_cfork(){
    u32 pid;
    struct exec_context *new_ctx = get_new_ctx();
    struct exec_context *ctx = get_current_ctx();
     /* Do not modify above lines
     * 
     * */   
     /*--------------------- Your code [start]---------------*/
     
    pid = new_ctx -> pid;
    *new_ctx = *ctx;
    new_ctx -> ppid = ctx->pid;
    new_ctx -> pid = pid;
    new_ctx -> pgd = os_pfn_alloc(OS_PT_REG);

    for (int i = MM_SEG_CODE; i<MAX_MM_SEGS; i++)
    {
    	u64 start = ctx->mms[i].start;
	u64 end = ctx->mms[i].end;
	u64 flags = ctx->mms[i].access_flags;
	for (; start<end; start += 4096)
	{
		page_walk(ctx->pgd, new_ctx -> pgd, start,flags);
		
	}
    
    }
    struct vm_area* curr = NULL, *head = NULL, *par_vm = ctx->vm_area;
    while( par_vm != NULL)
    {
    	struct vm_area* nvm = os_alloc(sizeof(struct vm_area));
	*nvm = *par_vm; 
	if (head == NULL)
	{
		head = nvm;
		curr = nvm;
	}
	else
	{

		curr->vm_next = nvm;
		curr = nvm;
	}
	par_vm = par_vm->vm_next;
    }
  //  printk("ab hoga game\n");
    new_ctx ->vm_area = head;
    curr = head;
    while (curr != NULL)
    {
    	u64 start = curr->vm_start;
	u64 end = curr->vm_end;
	u64 flags = curr->access_flags;
	for(;start<end; start += 4096)
	{
		page_walk(ctx->pgd, new_ctx -> pgd, start,flags); 
	}
    
	curr = curr -> vm_next;
    }

	new_ctx->regs.rax = 0;  
	
     /*--------------------- Your code [end] ----------------*/
    
     /*
     * The remaining part must not be changed
     */
    copy_os_pts(ctx->pgd, new_ctx->pgd);
    do_file_fork(new_ctx);
    setup_child_context(new_ctx);
    reset_timer();

    return pid;
}


/* Cow fault handling, for the entire user address space
 * For address belonging to memory segments (i.e., stack, data) 
 * it is called when there is a CoW violation in these areas. 
 */

long handle_cow_fault(struct exec_context *current, u64 vaddr, int access_flags)
{
        long retval = -1;
 //	printk("rip : %x\n", current->regs.entry_rip); 
//	if ((access_flags & PROT_WRITE) == 0) return -1;
	u64 mask9 = (1LL<<9) - 1;
	u64 mask12 = (1LL<<12)-1;
	u64 temp = vaddr;
	u64 L4off = (temp >>12)&mask9;
	temp >>= 12;
	u64 L3off = (temp>>9)&mask9;
	temp>>=9;
	u64 L2off = (temp>>9)&mask9;
	temp>>=9;
	u64 L1off = (temp>>9)&mask9;
	u64* L1pte = (u64 *)osmap(current -> pgd) + L1off;
	if ((access_flags&PROT_WRITE) != 0)
	{
		*L1pte |= 8;
	
	}	
	if (((*L1pte) & 1) == 0)
	{
		return retval;
	}
	u64* L2pte = (u64 *)osmap((*L1pte)>>12) + L2off;
	if ((access_flags&PROT_WRITE) != 0)
	{
		*L2pte |= 8;
	
	}	
	if (((*L2pte) & 1) == 0)
	{
		return retval;
	}
	u64* L3pte = (u64 *)osmap((*L2pte)>>12) + L3off;
	if ((access_flags&PROT_WRITE) != 0)
	{
		*L3pte |= 8;
	
	}	
	if (((*L3pte) & 1) == 0)
	{
		return retval;
	}
	u64* L4_pte = (u64 *)osmap((*L3pte)>>12) + L4off;
	temp = *L4_pte;
	s8 pp = get_pfn((temp>>12));
//	printk("pp: %d\n",pp);
	if (pp >2)
	{
		u64 pfn = os_pfn_alloc(USER_REG);
		*L4_pte = (pfn<<12);
		*L4_pte |= ((temp)&mask12);
	 	 put_pfn((temp>>12));
		u64* pp = (u64 *)osmap(pfn);
		u64* pq = (u64*)osmap((temp>>12));

		memcpy(pp,pq,4096);
	}
//	printk("l4 : %x, %x\n",L4_pte,*L4_pte); 
	int g = (*L4_pte)&8;	
	if ((access_flags&PROT_WRITE) != 0)
	{
		*L4_pte |= 8;
	
	}	
       	put_pfn((temp)>>12);	
	if(g == 0)invlpg(vaddr);

  	return 1;
}
