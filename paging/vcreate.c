/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	STATWORD ps;
  	disable(ps);

	int bs_id;

	if (get_bsm(&bs_id) == SYSERR) {
		kprintf("No free store");
		return SYSERR;
	}

	if (hsize <= 0 || hsize > 256) {
		kprintf("Wrong hsize");
		return SYSERR;
	}

	int pid = create(procaddr, ssize, priority, name, nargs, args);

	bsm_map(pid, 4096, bs_id, hsize);
	bsm_tab[bs_id].bs_sem = 1;

	proctab[pid].store = bs_id;
	proctab[pid].vhpno = 4096;
	proctab[pid].vhpnpages = hsize;
	proctab[pid].vmemlist->mnext = (struct mblock *) (4096 * NBPG);
	proctab[pid].vmemlist->mnext->mlen = hsize * NBPG;

	restore(ps);
	return OK;
}
