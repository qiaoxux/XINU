/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
	STATWORD ps;
  	disable(ps);

	int i;
	bs_map_t bsm_tab[NSTORES];

	for (int i = 0; i < NSTORES; i++) {
		bsm_tab[i].bs_status = BSM_UNMAPPED;
		bsm_tab[i].bs_pid = -1;
		bsm_tab[i].bs_vpno = -1;
		bsm_tab[i].bs_vpages = -1;
		bsm_tab[i].bs_sem = 0;
	}

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
	STATWORD ps;
  	disable(ps);

	int i;
	for (i = 0; i < NSTORES; i++) {
		if (bsm_tab[i].bs_status == BSM_UNMAPPED) {
			*avail = i;
			return OK;
		}
	}

	kprintf("No free store");
	restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{	
	STATWORD ps;
  	disable(ps);

	if (i < 0 || i > NSTORES) {
		kprintf("Wrong store index");
		return SYSERR;
	}

	bsm_tab[i].bs_status = BSM_UNMAPPED;
	bsm_tab[i].bs_pid = -1;
	bsm_tab[i].bs_vpno = -1;
	bsm_tab[i].bs_vpages = -1;
	bsm_tab[i].bs_sem = 0;

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
	STATWORD ps;
  	disable(ps);

	if(isbadpid(pid)) {
		kprintf("Wrong process id");
		return SYSERR;
	}

	int i;
	int vpno = (int) (vaddr / NBPG);
	for (i = 0; i < NSTORES; i++) {
		if (bsm_tab[i].bs_status == BSM_MAPPED && bsm_tab[i].bs_pid == pid && 
			bsm_tab[i].bs_vpno == vpno) {
			*store = i;
			*pageth = bsm_tab[i].bs_npages;
		}
	}

	kprintf("No such entry");
	restore(ps);
	return SYSERR;
}

/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
	STATWORD ps;
  	disable(ps);

	if(isbadpid(pid)) {
		kprintf("Wrong process id");
		return SYSERR;
	}

	if (source < 0 || source > NSTORES) {
		kprintf("Wrong source store index");
		return SYSERR;
	}

	if (bsm_tab[source].bs_status == BSM_UNMAPPED) {
		bsm_tab[source].bs_status = BSM_MAPPED;
		bsm_tab[source].bs_pid = pid;
		bsm_tab[source].bs_vpno = vpno;
		bsm_tab[source].bs_npages = npages;
		
		restore(ps);
		return OK;
	}

	return SYSERR;
}

/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
	STATWORD ps;
  	disable(ps);

	if(isbadpid(pid)) {
		kprintf("Wrong process id");
		return SYSERR;
	}

	int i;
	for (i = 0; i < NSTORES; i++) {
		if (bsm_tab[i].bs_status == BSM_MAPPED && bsm_tab[i].bs_pid == pid && 
			bsm_tab[i].bs_vpno == vpno) {
			bsm_tab[i].bs_status = BSM_UNMAPPED;
			bsm_tab[i].bs_pid = -1;
			bsm_tab[i].bs_vpno = -1;
			bsm_tab[i].bs_vpages = -1;
			bsm_tab[i].bs_sem = 0;
		}
	}

	restore(ps);
	return OK;
}
