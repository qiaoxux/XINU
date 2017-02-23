/* releaseall.c - releaseall */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <lock.h>
#include <stdio.h>

void updatepinh(int pid) {
	int ldes;
	int maxprio = -1;
	struct pentry *pptr;
	struct lentry *lptr;
	struct plentry *plptr;

	pptr = &proctab[pid];
	for (ldes = 0; ldes < NLOCKS; ldes++) {
		if ( (plptr = &(pptr->plocks[ldes]))->plstate == PLCURR )
			if ( (lptr = &locktab[ldes])->lprio > maxprio ) 
				maxprio = lptr->lprio;
	}

	pptr->pinh = (pptr->pprio > maxprio) ? 0 : maxprio;
}

int getmaxprio(struct lentry *lptr) {
	int maxprio = -1;
	int currentprio;
	int pid = q[lptr->lqtail].qprev;
	while (pid != lptr->lqhead) {
		currentprio = getrealprio(pid);
		if (currentprio > maxprio)
			maxprio = currentprio;
		pid = q[pid].qprev;
	}
	return maxprio;
}

int getrealprio(int pid) {
	struct pentry *pptr = &proctab[pid];
	return (pptr->pinh != 0) ? pptr->pinh: pptr->pprio;
}

void setlock(struct lentry *lptr, int lstate, int ltype, int lockcnt, int lprio) {
	lptr->lstate  = lstate;
	lptr->ltype   = ltype;
	lptr->lockcnt = lockcnt;
	lptr->lprio   = lprio;
}

void setplock(struct plentry *plptr, int plstate, int pltype, int plockret, unsigned long plstart) {
	plptr->plstate 	= plstate;
	plptr->pltype 	= pltype;
	plptr->plockret = plockret;
	plptr->plstart 	= plstart;
}

void setprocess(struct pentry *pptr, int pstate, int lockid) {
	pptr->pstate = pstate;
	pptr->lockid = lockid;
}