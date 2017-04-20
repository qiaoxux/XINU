#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/* requests a new mapping of npages with ID map_id */
int get_bs(bsd_t bs_id, unsigned int npages) {
	STATWORD ps;
  	disable(ps);

	if (bs_id < 0 || bs_id >= NSTORES) {
		kprintf("get_bs: wrong bs_id\n");
		return SYSERR;
	}

	if (npages <= 0 || npages > 256) {
		kprintf("get_bs: wrong npages\n");
		return SYSERR;
	}

	if (bsm_tab[bs_id].bs_status == BSM_MAPPED) {
		if (bsm_tab[bs_id].bs_private == 1 && bsm_tab[bs_id].bs_pid != currpid) {
			kprintf("get_bs: exclusive backing store, %d %d %d \n", bsm_tab[bs_id].bs_pid, bs_id, bsm_tab[bs_id].bs_private);
			return SYSERR;
		}

		if (bsm_tab[bs_id].bs_sem == 1 && bsm_tab[bs_id].bs_pid != currpid) {
			kprintf("get_bs: occupied backing store, %d %d %d \n", bsm_tab[bs_id].bs_pid, bs_id, bsm_tab[bs_id].bs_sem);
			return SYSERR;
		}
	} else {
		bsm_tab[bs_id].bs_status = BSM_MAPPED;
		bsm_tab[bs_id].bs_pid = currpid;
		bsm_tab[bs_id].bs_vpno = 0;
		bsm_tab[bs_id].bs_npages = npages;
	}

	restore(ps);
    return bsm_tab[bs_id].bs_npages;
}


