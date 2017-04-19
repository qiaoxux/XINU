/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm() {
	STATWORD ps;
  	disable(ps);

	int i;
	for (i = 0; i < NSTORES; i++) {
		bsm_tab[i].bs_status = BSM_UNMAPPED;
		bsm_tab[i].bs_pid = -1;
		bsm_tab[i].bs_vpno = 0;
		bsm_tab[i].bs_npages = 0;
		bsm_tab[i].bs_sem =	-1;

		bsm_tab[i].bs_nmapping = 0;
		bsm_tab[i].bs_private = 0;
		bsm_tab[i].bs_frames = NULL;
	}

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * init_bsm_for_process - initialize backing store map table for process
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsmap_for_process(bs_map_t *bsmap) {
	STATWORD ps;
  	disable(ps);

	int i;
	for (i = 0; i < NSTORES; ++i) {
		bsmap[i].bs_status = BSM_UNMAPPED;
		bsmap[i].bs_pid = -1;
		bsmap[i].bs_vpno = 0;
		bsmap[i].bs_npages = 0;
		bsmap[i].bs_sem =	-1;

		bsmap[i].bs_nmapping = 0;
		bsmap[i].bs_private = 0;
		bsmap[i].bs_frames = NULL;
	}
	
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail) {
	STATWORD ps;
  	disable(ps);

	int i;
	for (i = 0; i < NSTORES; i++) {
		if (bsm_tab[i].bs_status == BSM_UNMAPPED) {
			*avail = i;

			restore(ps);
			return OK;
		}
	}

	kprintf("No free store\n");
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i) {	
	STATWORD ps;
  	disable(ps);

	if (i < 0 || i > NSTORES) {
		kprintf("Wrong store index\n");
		return SYSERR;
	}

	bsm_tab[i].bs_status = BSM_UNMAPPED;
	bsm_tab[i].bs_pid = -1;
	bsm_tab[i].bs_vpno = 0;
	bsm_tab[i].bs_npages = 0;
	bsm_tab[i].bs_sem =	-1;

	bsm_tab[i].bs_nmapping = 0;
	bsm_tab[i].bs_private = 0;
	bsm_tab[i].bs_frames = NULL;
	
	proctab[currpid].bsmap[i].bs_status = BSM_UNMAPPED;
	proctab[currpid].bsmap[i].bs_pid = -1;
	proctab[currpid].bsmap[i].bs_vpno = 0;
	proctab[currpid].bsmap[i].bs_npages = 0;
	proctab[currpid].bsmap[i].bs_sem =	-1;

	proctab[currpid].bsmap[i].bs_nmapping = 0;
	proctab[currpid].bsmap[i].bs_private = 0;
	proctab[currpid].bsmap[i].bs_frames = NULL;

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vpno, int* store, int* pageth) {
	STATWORD ps;
  	disable(ps);

	if(isbadpid(pid)) {
		kprintf("Wrong process id\n");
		return SYSERR;
	}

	int i;
	bs_map_t *bs;
	for (i = 0; i < NSTORES; i++) {
		bs = &proctab[pid].bsmap[i];
		if (bs->bs_status == BSM_MAPPED && vpno >= bs->bs_vpno && 
			vpno < bs->bs_vpno + bs->bs_npages) {
			*store = i;
			*pageth = vpno - bs->bs_vpno;

			restore(ps);
			return OK;
		}
	}

	kprintf("No such entry\n");
	return SYSERR;
}

/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages) {
	STATWORD ps;
  	disable(ps);

	if(isbadpid(pid)) {
		kprintf("Wrong process id\n");
		return SYSERR;
	}

	if (vpno < 4096) {
		kprintf("Wrong virtual page number\n");
		return SYSERR;
	}

	if (source < 0 || source > NSTORES) {
		kprintf("Wrong source store index\n");
		return SYSERR;
	}

	if (npages <= 0 || npages > 256) {
		kprintf("Wrong npages\n");
		return SYSERR;
	}

	if (++bsm_tab[source].bs_nmapping == 1) {
		bsm_tab[source].bs_status = BSM_MAPPED;
		bsm_tab[source].bs_pid = pid;
		bsm_tab[source].bs_vpno = vpno;
		bsm_tab[source].bs_npages = npages;
	}
	
	proctab[pid].bsmap[source].bs_status = BSM_MAPPED;
	proctab[pid].bsmap[source].bs_vpno = vpno;
	proctab[pid].bsmap[source].bs_npages = npages;
	
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag) {
	STATWORD ps;
  	disable(ps);

  	int store, pageth;

	if(bsm_lookup(currpid, vpno, &store, &pageth) == SYSERR){
      	kprintf("bsm_unmap could not find mapping!\n");
      	return SYSERR;
  	}
 
  	decrease_frm_refcnt(pid, store);
  	bsm_tab[store].bs_nmapping--;
  
  	set_PDBR(pid);

  	restore(ps);
	return OK;
}
