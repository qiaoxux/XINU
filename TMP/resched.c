/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sched.h>
#include <math.h>

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);

void initepoch();	/* initepoch  --  initialize a new epoch */

/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */

int resched()
{
	register struct	pentry	*optr = &proctab[currpid];	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */
	
	/* if the old process is null process and ready queue is empty, reset preempt and return. */
	if (optr->pstate == PRCURR && isempty(rdyhead)) {
		#ifdef	RTCLOCK
			preempt = QUANTUM;
		#endif
		return OK;
	}
	
	double exprand;
	int nextpid, pid, newepoch, maxgoodness = -1;
	switch (getschedclass()) {
		case EXPDISTSCHED: /* Exponential Distribution Scheduler */
			exprand = expdev(0.1);

			/* select the process with the lowest priority that is greater than the random number in
			a round-robin way.
			*/

			nextpid = q[rdyhead].qnext;
			while (q[nextpid].qkey <= exprand) {
				if (q[nextpid].qnext == rdytail)
					break;
				nextpid = q[nextpid].qnext;	
			}

			while (q[nextpid].qkey == q[q[nextpid].qkey].qkey) {
				if (q[nextpid].qnext == rdytail)
					break;
				nextpid = q[nextpid].qnext;	
			}

			/* no switch needed if current process closer to exprand than nextpid*/

			if ( optr->pstate == PRCURR && 
				optr->pprio > exprand && optr->pprio < q[nextpid].qkey ) {
				#ifdef	RTCLOCK
					preempt = QUANTUM;
				#endif
				return(OK);
			}

			/* force context switch */

			if (optr->pstate == PRCURR) {
				optr->pstate = PRREADY;
				insert(currpid, rdyhead, optr->pprio);
			}

			/* remove nextpid process and mark it currently running */

			nptr = &proctab[ (currpid = dequeue(nextpid)) ];
			nptr->pstate = PRCURR;

			/* reset preemption counter	*/

			#ifdef	RTCLOCK
				preempt = QUANTUM;
			#endif

			ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
			/* The OLD process returns here when resumed. */
			return OK;
		case LINUXSCHED:	/* Linux-like Scheduler */
			optr->pcounter = preempt;	/* set preemption counter to process counter */

			/* calculate each process's goodness and decide whether to initialize a new epoch */

			newepoch = 1;
			for (pid = 1; pid < NPROC; pid++) {
				struct pentry *pptr = &proctab[pid];
				if ((pptr->pstate == PRCURR || pptr->pstate == PRREADY) && pptr->pnewprocess == 0) {
					if (pptr->pcounter == 0) 
						pptr->pgoodness = 0;
					else {
						pptr->pgoodness = pptr->pprio + pptr->pcounter;
						if (pptr->pgoodness > maxgoodness) {
							nextpid = pid;
							maxgoodness = pptr->pgoodness;
						}
						newepoch = 0;
					}
				}
			}

			/* initialize a new epoch, reset preempt and return OK */

			if (newepoch) {
				initepoch();
				#ifdef	RTCLOCK
					preempt = optr->pcounter;
				#endif
				return(OK);
			}

			/* no switch needed if current process goodness higher than next*/

			if ( (optr->pstate == PRCURR) && (optr->pgoodness >= maxgoodness)) {
				#ifdef	RTCLOCK
					preempt = optr->pcounter;
				#endif
				return(OK);
			}

			/* force context switch */

			if (optr->pstate == PRCURR) {
				optr->pstate = PRREADY;
				insert(currpid, rdyhead, optr->pprio);
			}

			/* remove nextpid process and mark it currently running */

			nptr = &proctab[ (currpid = dequeue(nextpid)) ];
			nptr->pstate = PRCURR;

			#ifdef	RTCLOCK
				preempt = nptr->pquantum;		/* reset preemption counter	*/
			#endif

			ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
			/* The OLD process returns here when resumed. */
			return OK;
		default:
			/* no switch needed if current process priority higher than next*/

			if ( (optr->pstate == PRCURR) &&
			   (lastkey(rdytail)<optr->pprio)) {
				return(OK);
			}
			
			/* force context switch */

			if (optr->pstate == PRCURR) {
				optr->pstate = PRREADY;
				insert(currpid,rdyhead,optr->pprio);
			}

			/* remove highest priority process at end of ready list */

			nptr = &proctab[ (currpid = getlast(rdytail)) ];
			nptr->pstate = PRCURR;		/* mark it currently running	*/

			#ifdef	RTCLOCK
				preempt = QUANTUM;		/* reset preemption counter	*/
			#endif

			ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
			/* The OLD process returns here when resumed. */
			return OK;
	}
}

/*-----------------------------------------------------------------------
 * initepoch  --  initialize a new epoch
 *
 * Notes:	update newprocess mark, priority, quantum, counter
 *------------------------------------------------------------------------
 */

void initepoch() {
	int pid;
	for (pid = 1; pid < NPROC; pid++) {
		struct pentry *pptr = &proctab[pid];
		pptr->pnewprocess = 0;
		pptr->pprio = pptr->pnewprio;
		pptr->pquantum = pptr->pprio + (int) (0.5 * pptr->pcounter);
		pptr->pcounter = pptr->pquantum;
	}

	return;
}
