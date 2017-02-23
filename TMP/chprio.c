/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;
	int i;
	struct	pentry	*pptr;
	struct  lentry	*lptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	pptr->pprio = newprio;
	if (pptr->pstate == PRREADY && pid != currpid) {
		dequeue(pid);
		insert(pid, rdyhead, newprio);
	}

	if (pptr->lockid != -1) {
		lptr = &locktab[pptr->lockid];
		lptr->lprio = getmaxprio(lptr);

		for (i = 0; i < NPROC; i++)
			if (lptr->lprocs[i] == 1)
				updatepinh(i);
	}

	restore(ps);
	return(newprio);
}

