#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <mark.h>
#include <bufpool.h>
#include <paging.h>

int write_bs(char *src, bsd_t bs_id, int page) {
	STATWORD ps;
  	disable(ps);

	if (bs_id < 0 || bs_id > 7) {
		kprintf("Wrong bs_id");
		return SYSERR;
	}

	if (page < 0 || page > 256) {
		kprintf("Wrong page number");
		return SYSERR;
	}

  	/* write one page of data from src to the backing store bs_id. */
   	char * phy_addr = BACKING_STORE_BASE + bs_id * BACKING_STORE_UNIT_SIZE + page * NBPG;
   	bcopy((void*)src, phy_addr, NBPG);

   	restore(ps);
   	return OK;
}

