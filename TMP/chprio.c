/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <sched.h>

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}

	if (getschedclass() == LINUXSCHED)
		pptr->pnewprio = newprio;
	else
		pptr->pprio = newprio;

	if (pptr->pstate == PRREADY && getschedclass() == EXPDISTSCHED && pid != currpid) {
		dequeue(pid);
		insert(pid, rdyhead, newprio);
	}

	restore(ps);
	return(newprio);
}
