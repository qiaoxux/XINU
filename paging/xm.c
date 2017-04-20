/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
	STATWORD ps;
  	disable(ps);

	if (virtpage < 4096) {
		kprintf("xmmap: wrong virtual page number\n");
		return SYSERR;
	}

  	if (source < 0 || source >= NSTORES) {
		kprintf("xmmap: wrong source\n");
		return SYSERR;
	}

	if (npages > bsm_tab[source].bs_npages || npages <= 0 || npages > 256) {
		kprintf("xmmap: wrong npages\n");
		return SYSERR;
	}

	if (bsm_tab[source].bs_private == 1) {
		kprintf("xmmap: virtual heap\n");
		return SYSERR;
	}

	if (bsm_tab[source].bs_status == BSM_UNMAPPED) {
		kprintf("xmmap: bs is not mapped\n");
 		return SYSERR;
	}

	if (bsm_map(currpid, virtpage, source, npages) == SYSERR) {
  		kprintf("xmmap could not create mapping!\n");
 		return SYSERR;
 	}

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
	STATWORD ps;
	disable(ps);

	int store, pageth;
  	unsigned long vaddr;
  	
  	if (virtpage < 4096) {
		kprintf("xmunmap: wrong virtual page number\n");
		return SYSERR;
	}
 	
 	if(bsm_unmap(currpid, virtpage, 0) == SYSERR){
      	kprintf("xmunmap: could not find mapping!\n");
      	return SYSERR;
  	}
  	
  	restore(ps);
  	return OK;
}
