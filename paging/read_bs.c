#include <conf.h>
#include <kernel.h>
#include <mark.h>
#include <bufpool.h>
#include <proc.h>

SYSCALL read_bs(char *dst, bsd_t bs_id, int page) {
	STATWORD ps;
  	disable(ps);

	if (bs_id < 0 || bs_id >= NSTORES) {
		kprintf("read_bs: wrong bs_id\n");
		return SYSERR;
	}

	if (page < 0 || page > 256) {
		kprintf("read_bs: wrong page number\n");
		return SYSERR;
	}

	/* fetch page from map map_id and write beginning at dst. */
   	void * phy_addr = BACKING_STORE_BASE + bs_id * BACKING_STORE_UNIT_SIZE + page * NBPG;
   	bcopy(phy_addr, (void*)dst, NBPG);


   	restore(ps);
   	return OK;
}


