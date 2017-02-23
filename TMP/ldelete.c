/* ldelete.c - ldelete */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <lock.h>
#include <stdio.h>

int ldelete(int lockdescriptor) {
	STATWORD ps;
	struct lentry *lptr;
	struct pentry *pptr;
	struct plentry *plptr;

	disable(ps);
	if (isbadlock(lockdescriptor) || ( (lptr = &locktab[lockdescriptor])->lstate==LFREE) ) {
		restore(ps);
		return(SYSERR);
	}

	/* reset lock */
	resetlock(lptr);
	
	/* delete the lock information in related processes		*/
	int pid;
	for (pid = 0; pid < NPROC; pid++) {
		pptr = &proctab[pid];
		switch (( plptr = &(pptr->plocks[lockdescriptor]) )->plstate) {
			case PLCURR:
				resetplock(plptr, SYSERR);
				break;
			case PLWAIT:
				resetplock(plptr, DELETED);
				pptr->lockid = -1;
				break;
			default:
				continue;
		}
	}

	restore(ps);
	return lockdescriptor;
}

void resetlock(struct lentry *lptr) {
	int pid;

	lptr->lstate = LFREE;
	lptr->ltype = 0;
	lptr->lockcnt = 0;
	lptr->lprio = -1;
	if (nonempty(lptr->lqhead))
		while( getfirst(lptr->lqhead) != EMPTY);
	for (pid = 0; pid < NPROC; pid++)
		lptr->lprocs[pid] = 0;
}

void resetplock(struct plentry *plptr, int plockret) {
	plptr->plstate = PLFREE;
	plptr->pltype = 0;
	plptr->plockret = plockret;
	plptr->plstart = 0;
}