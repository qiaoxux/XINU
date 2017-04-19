#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/* requests a new mapping of npages with ID map_id */
int get_bs(bsd_t bs_id, unsigned int npages) {
	if (bs_id < 0 || bs_id >= NSTORES) {
		kprintf("get_bs: wrong bs_id\n");
		return SYSERR;
	}

	if (npages <= 0 || npages > 256) {
		kprintf("get_bs: wrong npages\n");
		return SYSERR;
	}
	kprintf("get_bs: %d %d %d %d\n", currpid, bs_id, npages);
	if (bsm_tab[bs_id].bs_status == BSM_MAPPED) {
		if (bsm_tab[bs_id].bs_private = 1 && bsm_tab[bs_id].bs_pid != currpid) {
			kprintf("get_bs: exclusive backing store\n");
			return SYSERR;
		}

		if (bsm_tab[bs_id].bs_sem = 1 && bsm_tab[bs_id].bs_pid != currpid) {
			kprintf("get_bs: occupied backing store\n");
			return SYSERR;
		}
	} else {
		kprintf("get_bs: %d %d %d %d\n", currpid, bs_id, npages);
		bsm_tab[bs_id].bs_status = BSM_MAPPED;
		kprintf("get_bs: %d %d %d %d\n", currpid, bs_id, npages);
		bsm_tab[bs_id].bs_pid = currpid;
		kprintf("get_bs: %d %d %d %d\n", currpid, bs_id, npages);
		bsm_tab[bs_id].bs_npages = npages;
	}

	kprintf("get_bs: %d %d %d %d\n", currpid, bs_id, npages);
    return bsm_tab[bs_id].bs_npages;
}


