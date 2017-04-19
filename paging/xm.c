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
		kprintf("Wrong virtual page number\n");
		return SYSERR;
	}

  	if (source < 0 || source > 7) {
		kprintf("Wrong source\n");
		return SYSERR;
	}

	if (npages <= 0 || npages > 256) {
		kprintf("Wrong npages\n");
		return SYSERR;
	}

	bsm_map(currpid, virtpage, source, npages);

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

	if (virtpage < 4096) {
		kprintf("Wrong virtual page number\n");
		return SYSERR;
	}

	bsm_unmap(currpid, virtpage, 0);
	
	restore(ps);
  	return OK;
}
