/* releaseall.c - releaseall */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <lock.h>
#include <stdio.h>

int releaseall(numlocks, args)
int numlocks;
long args;
{
	STATWORD ps;
	int i;
	int ldes;
	int err = 0;
	unsigned long *a;		/* points to list of args	*/
	struct pentry *pptr;
	struct lentry *lptr;
	struct plentry *plptr;

	struct pentry *newpptr;
	struct plentry *newplptr;

	disable(ps);

	a = (unsigned long *)(&args) + (numlocks - 1); /* last argument	*/
	for ( ; numlocks > 0 ; numlocks--) {
		ldes = *a--; 	/* lock descriptor */

		lptr = &locktab[ldes];
		pptr = &proctab[currpid];
		if (( plptr = &(pptr->plocks[ldes]) )->plstate != PLCURR) {
			err = 1;
			continue;
		}

		/* update lock and process information */
		lptr->lprocs[currpid] = 0;
		setlock(lptr, LUSED, lptr->ltype, (lptr->ltype == WRITE) ? 0 : lptr->lockcnt + 1, getmaxprio(lptr));
		setplock(plptr, PLFREE, 0, OK, 0);
		for (i = 0; i < NPROC; i++)
			if (lptr->lprocs[i] == 1)
				updatepinh(i);

		int newpid, maxprio, maxtime, maxtype;
		int prevpid, prevtime, prevtype;
		/* wait queue is not empty */
		if (nonempty(lptr->lqhead)) {
			if (lptr->lockcnt == 0) {
				newpid = q[lptr->lqtail].qprev;				
				maxtype = proctab[newpid].plocks[ldes].pltype;
				if (maxtype == READ) {
					prevpid = newpid;
					maxprio = getrealprio(newpid);
					maxtime = ctr1000 - proctab[newpid].plocks[ldes].plstart;
					while(q[prevpid].qprev != lptr->lqhead) {
						prevtime = ctr1000 - proctab[q[prevpid].qprev].plocks[ldes].plstart;
						prevtype = proctab[q[prevpid].qprev].plocks[ldes].pltype;
						if (maxprio > getrealprio(q[prevpid].qprev) || abs(maxtime - prevtime) > 1000)
							break;
						if (prevtype == WRITE) {
							newpid = q[prevpid].qprev;
							maxtype = prevtype;
							break;
						}
						prevpid = q[prevpid].qprev;
					}
				}

				newpptr = &proctab[newpid];
				newplptr = &(newpptr->plocks[ldes]);

				dequeue(newpid);

				lptr->lprocs[newpid] = 1;
				setlock(lptr, LUSED, maxtype, (maxtype == READ) ? lptr->lockcnt - 1 : 1, getmaxprio(lptr));
				setplock(newplptr, PLCURR, maxtype, OK, 0);
				setprocess(newpptr, PRREADY, -1);
				for (i = 0; i < NPROC; i++)
					if (lptr->lprocs[i] == 1)
						updatepinh(i);
				ready(newpid, RESCHNO);
			}

			if (lptr->lockcnt < 0) {
				int tmppid;

				prevpid = q[lptr->lqtail].qprev;
				while(prevpid != lptr->lqhead && proctab[prevpid].plocks[ldes].pltype == READ) {
					tmppid = q[prevpid].qprev;
					newpptr = &proctab[prevpid];
					newplptr = &(newpptr->plocks[ldes]);

					dequeue(prevpid);

					lptr->lprocs[prevpid] = 1;
					setlock(lptr, LUSED, READ, lptr->lockcnt - 1, getmaxprio(lptr));
					setplock(newplptr, PLCURR, READ, OK, 0);
					setprocess(newpptr, PRREADY, -1);
					for (i = 0; i < NPROC; i++)
						if (lptr->lprocs[i] == 1)
							updatepinh(i);
					ready(newpid, RESCHNO);
					prevpid = tmppid;
				}
			}
		} else {
			lptr->ltype = 0;
			lptr->lprio = -1;
		}
	}

	resched();

	if (err) {
		restore(ps);
		return(SYSERR);	
	}
	
	restore(ps);
	return OK;
}