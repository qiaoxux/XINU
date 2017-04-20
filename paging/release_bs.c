#include <conf.h>
#include <kernel.h>
#include <proc.h>

/* release the backing store with ID bs_id */
SYSCALL release_bs(bsd_t bs_id) {
	STATWORD ps;
  	disable(ps);

	if (bs_id < 0 || bs_id >= NSTORES) {
		kprintf("Wrong bs_id");
		return SYSERR;
	}
  	
 	if (free_bsm(bs_id) == SYSERR) {
 		return SYSERR;
 	}

 	restore(ps);
   	return OK;
}