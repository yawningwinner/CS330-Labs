#include<context.h>
#include<page.h>
#include<memory.h>
#include<lib.h>

//const struct exec_context *head = rr_list_head;
//////////////////////  Q1: RR Scheduling   ///////////////////////////////////////
//args:
//      ctx: new exec_context to be added in the linked list
void rr_add_context(struct exec_context *ctx)
{
      /*TODO*/
      struct exec_context* p = rr_list_head;
      if(p==NULL) {
      	rr_list_head = ctx;
	rr_list_head->next = NULL;
	return;
      }
      while(p->next!=NULL)p = p->next;
       p->next = ctx;
       ctx->next = NULL;
       //printk("process added : %x\n",ctx);
       return;
}

//args:
//      ctx: exec_context to be removed from the linked list
void rr_remove_context(struct exec_context *ctx)
{
	/*TODO*/
	// printk("something happen??\n");
	struct exec_context *temp = rr_list_head,*prev = rr_list_head;
	if(temp==ctx) {
		rr_list_head = ctx->next;
		return;
	}
	while(temp!=ctx) {
		prev = temp;
		temp = temp->next;
	}
	prev->next = temp->next;
	return;
}

//args:
//      ctx: exec_context corresponding the currently running process
struct exec_context *rr_pick_next_context(struct exec_context *ctx)
{
    /*TODO*/
     //printk("whaat?\n");
     if(ctx->next!=NULL)return ctx->next;
     if(rr_list_head!=NULL)return rr_list_head; 
    // printk("swapper\n");
     return get_ctx_by_pid(0);
}

//////////////////////  Q2: Get the PAGE TABLE details for given address   ///////////////////////////////////////


//args:
//      ctx: exec_context corresponding the currently running process
//      addr: address for which the PAGE TABLE details are to be printed

int do_walk_pt(struct exec_context *ctx, unsigned long addr)
{
    u64 *vaddr_base = (u64 *)osmap(ctx->pgd);
    /*TODO*/
    u64 L1off = (addr & PGD_MASK) >> PGD_SHIFT;
    u64* L1addr = vaddr_base+ L1off;
    if((*L1addr)&1==0) {
    	printk("No L2 entry\n");
    	printk("No L3 entry\n");
    	printk("No L4 entry\n");
	return -1;
    }
    u64 L1phy = *L1addr;
    u64 L1flag = L1phy&0xFFF;
    L1phy = L1phy>>12;
    printk("L1-entry addr: %x, L1-entry contents: %x, PFN: %x, Flags: %x\n",vaddr_base,*L1addr,L1phy,L1flag);
    // L2
    vaddr_base = (u64*)osmap(L1phy);
    u64 L2off = (addr & PUD_MASK) >> PUD_SHIFT;
    u64* L2addr = vaddr_base + L2off;
    u64 L2_entry = *L2addr;
    if((L2_entry&1)==0) {
	printk("NO L2 entry\n");
    	printk("No L3 entry\n");
    	printk("No L4 entry\n");
	return -1;
    }
    u64 L2_Phy = L2_entry>>12;
    u64 L2_flags = L2_entry &0xFFF;
    
    printk("L2-entry addr: %x, L2-entry contents: %x, PFN: %x, Flags: %x\n",vaddr_base+L2off,*L2addr,L2_Phy,L2_flags);
    // L3
    vaddr_base = (u64*)osmap(L2_Phy);
    u64 L3off = (addr & PMD_MASK) >> PMD_SHIFT;
    u64* L3addr = vaddr_base + L3off;
    u64 L3_entry = *L3addr;
    if((L3_entry&1)==0) {
    	printk("No L3 entry\n");
    	printk("No L4 entry\n");
	return -1;
    }
    u64 L3_Phy = L3_entry>>12;
    u64 L3_flags = L3_entry &0xFFF;
    
    printk("L3-entry addr: %x, L3-entry contents: %x, PFN: %x, Flags: %x\n",vaddr_base+L3off,*L3addr,L3_Phy,L3_flags);
    // L4
    vaddr_base = (u64*)osmap(L3_Phy);
    u64 L4off = (addr & PTE_MASK) >> PTE_SHIFT;
    u64* L4addr = vaddr_base + L4off;
    u64 L4_entry = *L4addr;
    if((L4_entry&1)==0) {
    //	printk("No L3 entry\n");
    	printk("No L4 entry\n");
	return -1;
    }
    u64 L4_Phy = L4_entry>>12;
    u64 L4_flags = L4_entry &0xFFF;
    
    printk("L4-entry addr: %x, L4-entry contents: %x, PFN: %x, Flags: %x\n",vaddr_base+L4off,*L4addr,L4_Phy,L4_flags);

    return 0;
}

