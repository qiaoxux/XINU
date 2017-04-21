/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
	unsigned nbytes;
{
	STATWORD ps;

	disable(ps);
	struct pentry *pptr = &proctab[currpid];
	if (nbytes == 0 ) {
		restore(ps);
		return ((WORD *) SYSERR);
	}
	mem_list *tmp = &(pptr->mem_list_t);
	while(tmp != NULL){
		if(tmp->memlen == nbytes){
			tmp->memlen = 0;
			return ((WORD *) tmp->mem);
		}
		else if(tmp->memlen > nbytes){
			char *loc = tmp->mem;
			tmp->mem = (unsigned)tmp->mem + nbytes;
			tmp->memlen -= nbytes;
			return ((WORD *) tmp->mem);
		}
		tmp = tmp->next;
	}
	restore(ps);
	return ((WORD *) SYSERR);

}