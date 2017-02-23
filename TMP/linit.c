/* linit.c - linit */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <lock.h>
#include <stdio.h>

void linit() {
	int i;
	struct lentry *lptr;
	
	nextlock = NLOCKS-1;
	for (i = 0; i < NLOCKS; i++) {	/* initialize locks */
		(lptr = &locktab[i])->lstate = LFREE;
		lptr->lqtail = 1 + (lptr->lqhead = newqueue());
	}
}