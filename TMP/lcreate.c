/* lcreate.c - lcreate, resetlock */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <lock.h>
#include <stdio.h>

int lcreate(void) {
	STATWORD ps;
	int	i;
	int	lockdescriptor;			/* lock descriptor to return		*/
	struct lentry *lptr;

	disable(ps);
	for (i=0 ; i<NLOCKS ; i++) {	/* check all NLOCKS slots	*/
		if ( (lockdescriptor=nextlock--) <= 0 )
			nextlock = NLOCKS-1;
		if ( (lptr = &locktab[lockdescriptor])->lstate == LFREE) {
			initlock(lptr);
			restore(ps);
			return lockdescriptor;
		}
	}
	restore(ps);
	return(SYSERR);
}

void initlock(struct lentry *lptr) {
	int pid;

	lptr->lstate = LUSED;
	lptr->ltype = 0;
	lptr->lockcnt = 0;
	lptr->lprio = -1;
	for (pid = 0; pid < NPROC; pid++)
		lptr->lprocs[pid] = 0;
}