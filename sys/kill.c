/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	kprintf("You are killing pid %d\n", pid);
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev, i;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);

	for(i = 0; i < NFRAMES; i++) {
		if(frm_tab[i].fr_status == FRM_MAPPED && frm_tab[i].fr_pid == pid) {
			free_frm(i, pid);
		}
	}

	for(i = 0; i < 8; i++) {
		if(proctab[pid].bsmap[i].bs_status == BSM_MAPPED) {
			if(--bsm_tab[i].bs_nmapping == 0) {
				bsm_tab[i].bs_status = BSM_UNMAPPED;
				bsm_tab[i].bs_pid = -1;
				bsm_tab[i].bs_vpno = 0;
				bsm_tab[i].bs_npages = 0;
				bsm_tab[i].bs_private = 0;
			}

		  	proctab[pid].bsmap[i].bs_status = BSM_UNMAPPED;
			proctab[pid].bsmap[i].bs_vpno = 0;
			proctab[pid].bsmap[i].bs_npages = 0;

			

		}
	}

	if (proctab[pid].bsmap[i].bs_private == 1) {
		bsm_tab[i].bs_private = 0;
		proctab[pid].bsmap[i].bs_private == 0;
	}
	

	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}

	restore(ps);
	return(OK);
}
