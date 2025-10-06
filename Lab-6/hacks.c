#include<hacks.h>
#include<sigexit.h>
#include<entry.h>
#include<lib.h>
#include<context.h>
#include<memory.h>

struct hack_config{
	             long cur_hack_config;
		     u64 usr_handler_addr;
};

//Used to store the current hack configuration
static struct hack_config hconfig = {-1, -1};


//system call handler to configure the hack sematics
//user space connection is already created, You need to
//store the hack semantics in the 'hconfig' structure (after validation)
//which will be used when division-by-zero occurs

long sys_config_hs(struct exec_context *ctx, long hack_mode, void *uhaddr)
{	
	/* if(hack_mode<DIV_ZERO_OPER_CHANGE || hack_mode > DIV_ZERO_MAX) {
		printk("Error...exiting\n");
		do_exit(0);
	} */
   	switch (hack_mode) {
		case DIV_ZERO_OPER_CHANGE :
			hconfig.cur_hack_config = hack_mode;
			hconfig.usr_handler_addr = (u64)uhaddr;
			break;
		case DIV_ZERO_SKIP :
			hconfig.cur_hack_config = hack_mode;
			hconfig.usr_handler_addr = (u64)uhaddr;
	//		printk("regs addr : %x, os_rsp addr: %x\n",(u64)&ctx->regs,ctx->os_rsp);
			break;
		case DIV_ZERO_USH_EXIT :
			hconfig.cur_hack_config = hack_mode;
			hconfig.usr_handler_addr = (u64)uhaddr;
			if((u64)uhaddr<ctx->mms[MM_SEG_CODE].start || (u64)uhaddr>ctx->mms[MM_SEG_CODE].end){
			   
			hconfig.cur_hack_config = -1;
			hconfig.usr_handler_addr = -1;
			return -EINVAL;
			
			}
			break;
		case DIV_ZERO_SKIP_FUNC :
			hconfig.cur_hack_config = hack_mode;
			hconfig.usr_handler_addr = (u64)uhaddr;
			break;
		case DIV_ZERO_MAX :
			hconfig.cur_hack_config = hack_mode;
			hconfig.usr_handler_addr = (u64)uhaddr;
			break;

		default :
			hconfig.cur_hack_config = -1;
			hconfig.usr_handler_addr = -1;
			return -EINVAL;
	}
       	return 0; 
}


/*This is the handler for division by zero
 * 'regs' is a structure defined in include/context.h which
 * is already filled with the user execution state (by the asm handler)
 * and will be restored back when the function returns 
 *
 */
int do_div_by_zero(struct user_regs *regs)
{
    switch (hconfig.cur_hack_config) {
    	case DIV_ZERO_OPER_CHANGE :
		regs->rax = 0;
		regs->rcx = 1;
		break;
	case DIV_ZERO_SKIP :
		regs->entry_rip +=3;
		break;
	case DIV_ZERO_USH_EXIT :
		regs->rdi = regs->entry_rip;
		regs->entry_rip = hconfig.usr_handler_addr;
		break;
	case DIV_ZERO_SKIP_FUNC :
		regs->entry_rip = *((u64*)(regs->rbp+0x8));
		regs->entry_rsp = regs->rbp+0x10;
		regs->rbp =*((u64*)(regs->rbp));
		regs->rax=1;
		break;
	default :	
		printk("Error...exiting\n");
		do_exit(0);
    }	
    return 0;   	
}
