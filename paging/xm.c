/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
  STATWORD ps;
  disable(ps);

	if (bsm_map(currpid, virtpage, source, npages) == SYSERR) {
  		kprintf("xmmap could not create mapping!\n");
  		restore(ps);
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
 	
 	if(bsm_unmap(currpid, virtpage, 0) == SYSERR){
    	kprintf("xmunmap: could not find mapping!\n");
    	restore(ps);
    	return SYSERR;
	}
	
	restore(ps);
	return OK;
}
