#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/* requests a new mapping of npages with ID map_id */
int get_bs(bsd_t bs_id, unsigned int npages) {
	if (bs_id < 0 || bs_id > 7) {
		kprintf("Wrong bs_id\n");
		return SYSERR;
	}

	if (npages <= 0 || npages > 256) {
		kprintf("Wrong npages\n");
		return SYSERR;
	}

	if (bsm_tab[bs_id].bs_status == BSM_MAPPED) {
		if (bsm_tab[bs_id].bs_sem = 1 && bsm_tab[bs_id].bs_pid != currpid) {
			kprintf("Exclusive backing store\n");
			return SYSERR;
		}
	} else {
		bsm_tab[bs_id].bs_status = BSM_MAPPED;
		bsm_tab[bs_id].bs_pid = currpid;
		bsm_tab[bs_id].bs_npages = npages;
	}

    return bsm_tab[bs_id].bs_npages;
}


