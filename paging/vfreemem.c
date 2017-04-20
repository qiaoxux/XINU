/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(block, size)
	struct	mblock	*block;
	unsigned size;
{	 
	STATWORD ps;
	disable(ps);

	struct	mblock	*p, *q;
	unsigned top;

	struct	mblock *vmemlist;
	vmemlist = proctab[currpid].vmemlist;
	
	if (size == 0 || size > 256 * NBPG)
		return(SYSERR);
	if ( (unsigned) block < (unsigned) 4096 * NBPG || (unsigned) block > (unsigned)(proctab[currpid].vhpno + proctab[currpid].vhpnpages)*NBPG)
		return(SYSERR);

	size = (unsigned)roundmb(size);

	for( p=(vmemlist->mnext),q=vmemlist; p != (struct mblock *) NULL && p < block; q = p, p = p->mnext );
	if (((top=q->mlen+(unsigned)q)>(unsigned)block && q!= vmemlist) ||
	    (p!=NULL && (size+(unsigned)block) > (unsigned)p )) {
		restore(ps);
		return(SYSERR);
	}

	if ( q!= vmemlist && top == (unsigned)block ) {
		
		q->mlen += size;
	}
	else {
		block->mlen = size;
		block->mnext = p;
		q->mnext = block;
		q = block;
	}

	if ( (unsigned)( q->mlen + (unsigned)q ) == (unsigned)p) {
		q->mlen += p->mlen;
		q->mnext = p->mnext;
	}

	restore(ps);
	return(OK);
}