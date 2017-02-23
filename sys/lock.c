/* lock.c - lock */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <lock.h>
#include <stdio.h>

int lock(int ldes1, int type, int priority) {
	STATWORD ps;
	int i;
	struct pentry *pptr;
	struct lentry *lptr;
	struct plentry *plptr;

	disable(ps);
	
	pptr = &proctab[currpid];
	plptr = &(pptr->plocks[ldes1]);
	
	if (isbadlock(ldes1)) {
		restore(ps);
		return(SYSERR);
	} 
	if ((lptr = &locktab[ldes1])->lstate == LFREE) {
		if (plptr->plockret == DELETED) {
			restore(ps);
			return(DELETED);
		}
		restore(ps);
		return(SYSERR);
	}
	if (plptr->plockret == DELETED) {
		restore(ps);
		return(SYSERR);
	}
	
	int pwait = 0;
	switch (lptr->ltype) {
		case WRITE:
			pwait = 1;
			break;
		case READ:
			if (type == READ) {
				int findwriter = 0;
				int prevpid = q[lptr->lqtail].qprev;
				while (prevpid != lptr->lqhead) {
					if (proctab[prevpid].plocks[ldes1].pltype == WRITE) {
						findwriter = 1;
						break;
					}
					prevpid = q[prevpid].qprev;
				}
				pwait = ( (findwriter && q[prevpid].qkey <= priority) || !findwriter) ? 0 : 1;
			} else {
				pwait = 1;
			}
			break;
		default:
			pwait = 0;
			break;
	}

	if (pwait) {
		insert(currpid, lptr->lqhead, priority);
		setlock(lptr, LUSED, lptr->ltype, lptr->lockcnt, getmaxprio(lptr));
		setplock(plptr, PLWAIT, type, OK, ctr1000);
		setprocess(pptr, PRWAIT, ldes1);
		for (i = 0; i < NPROC; i++)
			if (lptr->lprocs[i] == 1)
				updatepinh(i);
		resched();
		restore(ps);
		return OK;
	} else {
		lptr->lprocs[currpid] = 1;
		setlock(lptr, LUSED, type, (type == READ) ? lptr->lockcnt - 1 : 1, getmaxprio(lptr));
		setplock(plptr, PLCURR, type, OK, 0);
		setprocess(pptr, PRCURR, pptr->lockid);
		for (i = 0; i < NPROC; i++)
			if (lptr->lprocs[i] == 1)
				updatepinh(i);
		restore(ps);
		return OK;
	}

	restore(ps);
	return SYSERR;
}