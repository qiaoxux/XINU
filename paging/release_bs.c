#include <conf.h>
#include <kernel.h>
#include <proc.h>

/* release the backing store with ID bs_id */
SYSCALL release_bs(bsd_t bs_id) {
	STATWORD ps;
  	disable(ps);

	if (bs_id < 0 || bs_id >= NSTORES) {
		restore(ps);
		return SYSERR;
	}

	if (bsm_tab[bs_id].bs_nmapping != 0) {
		restore(ps);
		return SYSERR;
	}
  	
 	if (free_bsm(bs_id) == SYSERR) {
 		restore(ps);
 		return SYSERR;
 	}

 	restore(ps);
   	return OK;
}