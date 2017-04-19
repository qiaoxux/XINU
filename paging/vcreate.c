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

  	int pid = create(procaddr, ssize, priority, name, nargs, args);

	int bs_id;

	if (get_bsm(&bs_id) == SYSERR) {
		kprintf("No free store");
		return SYSERR;
	}

	if (hsize <= 0 || hsize > 256) {
		kprintf("Wrong hsize");
		return SYSERR;
	}
	
	bsm_tab[bs_id].bs_private = 1;
	proctab[pid].bsmap[bs_id].bs_private = 1;

	proctab[pid].vhpno = 4096;
	proctab[pid].vhpnpages = hsize;
	bsm_map(pid, 4096, bs_id, hsize);

	proctab[pid].vmemlist = getmem(sizeof(struct mblock *));
	proctab[pid].vmemlist->mnext = (struct mblock *) (vno2p(4096));
	proctab[pid].vmemlist->mlen = 0;

	struct mblock * memblock = bs2p(store);
    memblock->mnext = 0;  
    memblock->mlen  = hsize*NBPG;

	restore(ps);
	return OK;
}
