#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <mark.h>
#include <bufpool.h>

int write_bs(char *src, bsd_t bs_id, int page) {
	STATWORD ps;
  	disable(ps);

	if (bs_id < 0 || bs_id >= NSTORES) {
		kprintf("write_bs: wrong bs_id\n");
		return SYSERR;
	}

	if (page < 0 || page > 256) {
		kprintf("write_bs: wrong page number\n");
		return SYSERR;
	}

  	/* write one page of data from src to the backing store bs_id. */
   	char * phy_addr = BACKING_STORE_BASE + bs_id * BACKING_STORE_UNIT_SIZE + page * NBPG;
   	bcopy((void*)src, phy_addr, NBPG);

   	restore(ps);
   	return OK;
}

