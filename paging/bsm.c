/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
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
SYSCALL init_bsmap_for_process(int pid) {
	STATWORD ps;
  	disable(ps);

	int i;
	for (i = 0; i < NSTORES; i++) {
		proctab[pid].bsmap[i].bs_status = BSM_UNMAPPED;
		proctab[pid].bsmap[i].bs_pid = -1;
		proctab[pid].bsmap[i].bs_vpno = 0;
		proctab[pid].bsmap[i].bs_npages = 0;

		proctab[pid].bsmap[i].bs_nmapping = 0;
		proctab[pid].bsmap[i].bs_private = 0;
		proctab[pid].bsmap[i].bs_frames = NULL;
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

	kprintf("get_bsm: no free store\n");
	restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i) {	
	STATWORD ps;
  	disable(ps);

	if (i < 0 || i >= NSTORES) {
		kprintf("free_bsm: wrong store index\n");
		return SYSERR;
	}

	bsm_tab[i].bs_status = BSM_UNMAPPED;
	bsm_tab[i].bs_pid = -1;
	bsm_tab[i].bs_vpno = 0;
	bsm_tab[i].bs_npages = 0;

	bsm_tab[i].bs_nmapping = 0;
	bsm_tab[i].bs_private = 0;
	bsm_tab[i].bs_frames = NULL;
	
	proctab[currpid].bsmap[i].bs_status = BSM_UNMAPPED;
	proctab[currpid].bsmap[i].bs_pid = -1;
	proctab[currpid].bsmap[i].bs_vpno = 0;
	proctab[currpid].bsmap[i].bs_npages = 0;

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
		kprintf("bsm_lookup: wrong process id\n");
		restore(ps);
		return SYSERR;
	}

	int i;
	bs_map_t *bsmap;
	for (i = 0; i < NSTORES; i++) {
		bsmap = &proctab[pid].bsmap[i];
		// kprintf("bsm_lookup: %d %d %d %d %d \n", pid, i, bsmap->bs_status, bsmap->bs_vpno, bsmap->bs_npages);

		if (bsmap->bs_status == BSM_MAPPED && vpno >= bsmap->bs_vpno && 
			vpno < bsmap->bs_vpno + bsmap->bs_npages) {
			*store = i;
			*pageth = vpno - bsmap->bs_vpno;

			restore(ps);
			return OK;
		}
	}

	restore(ps);
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
		kprintf("bsm_map: wrong process id\n");
		restore(ps);
		return SYSERR;
	}

	if (vpno < 4096) {
		kprintf("bsm_map: wrong virtual page number\n");
		restore(ps);
		return SYSERR;
	}

	if (source < 0 || source >= NSTORES) {
		kprintf("bsm_map: wrong source store index\n");
		restore(ps);
		return SYSERR;
	}

	if (npages <= 0 || npages > 256) {
		kprintf("bsm_map: wrong npages\n");
		restore(ps);
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

  	kprintf("bsm_unmap: %d %d\n", pid, vpno);

  	int store, pageth;

	if(bsm_lookup(pid, vpno, &store, &pageth) == SYSERR){
      	kprintf("bsm_unmap: could not find mapping!\n");
      	restore(ps);
      	return SYSERR;
  	}
 
  	decrease_frm_refcnt(pid, store);

  	if (--bsm_tab[store].bs_nmapping == 0)
  		release_bs(store);
  
  	set_PDBR(pid);

  	restore(ps);
	return OK;
}
