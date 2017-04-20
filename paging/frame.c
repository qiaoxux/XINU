/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm() {
	STATWORD ps;
  	disable(ps);

	int i;
	fr_map_t frm_tab[NFRAMES];

	for (i = 0; i < NFRAMES; i++) {
		frm_tab[i].fr_status = FRM_UNMAPPED;
		frm_tab[i].fr_pid = -1;
		frm_tab[i].fr_vpno = -1;
		frm_tab[i].fr_refcnt = 0;
		frm_tab[i].fr_type = -1;
		frm_tab[i].fr_dirty = 0;

		frm_tab[i].fr_upper = -1;
		frm_tab[i].fr_age = 0;
	}

	struct qent pq[NPQ];

	pnewqueue();
	currqueue = HeadPQ;

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail) {
	STATWORD ps;
  	disable(ps);

	int i;
	for (i = 0; i < NFRAMES; i++) {
		if (frm_tab[i].fr_status == FRM_UNMAPPED) {
			*avail = i;

			restore(ps);
			return OK;
		}
	}

	pt_t *pt;
	if (grpolicy() == SC) {
		while (1) {
			if (currqueue == HeadPQ || currqueue == TailPQ)
				currqueue = pq[HeadPQ].qnext;
			
			if (frm_tab[currqueue].fr_type == FR_TBL) {
				pt = (pt_t*)fr2p(i);
				if (pt->pt_acc == 1) {
					pt->pt_acc = 0;
				} else {
					free_frm(currqueue, currpid);
					*avail = currqueue;

					restore(ps);
					return OK;
				}
			}

			currqueue = pq[currqueue].qnext;
		}
	} else {
		int min_age = 256;
		int frame_idx = pq[TailPQ].qprev, fit_frame;
		while (frame_idx != HeadPQ) {
			frm_tab[frame_idx].fr_age = (int) (frm_tab[frame_idx].fr_age/2);
			frame_idx = pq[frame_idx].qprev;
		}

		frame_idx = pq[TailPQ].qprev;
		fit_frame = frame_idx;
		while (frame_idx != HeadPQ) {
			if (frm_tab[frame_idx].fr_age <= min_age) {
				min_age = frm_tab[frame_idx].fr_age;
				fit_frame = frame_idx;
			}
			frame_idx = pq[frame_idx].qprev;
		}

		free_frm(fit_frame, currpid);
		*avail = fit_frame;
		
		restore(ps);
		return OK;
	}

	restore(ps);
	return SYSERR;
}

/*-------------------------------------------------------------------------
 * set_frm - initialize a frame after get_frm
 *-------------------------------------------------------------------------
 */
SYSCALL set_frm(int i, int pid, int type) {
	STATWORD ps;
    disable(ps);

	frm_tab[i].fr_status = FRM_MAPPED;
	frm_tab[i].fr_pid = pid;
	frm_tab[i].fr_type = type;

	frm_tab[i].fr_refcnt++;

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * reset_frm - reset a frame when free them
 *-------------------------------------------------------------------------
 */
SYSCALL reset_frm(int i) {
	STATWORD ps;
    disable(ps);

    frm_tab[i].fr_status = FRM_UNMAPPED;
	frm_tab[i].fr_pid = -1;
	frm_tab[i].fr_vpno = -1;
	frm_tab[i].fr_refcnt = 0;
	frm_tab[i].fr_type = -1;
	frm_tab[i].fr_dirty = 0;

	frm_tab[i].fr_upper = -1;
	frm_tab[i].fr_age = 0;

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i, int pid) {
	STATWORD ps;
    disable(ps);

    int upper, store, pageth;

	pt_t *pt_e;
	pd_t *pd_e;
	
	pt_e = (pt_t*) fr2p(i);
	pd_e = (pd_t*) fr2p(i);
	upper = frm_tab[i].fr_upper;
	
	if(frm_tab[i].fr_type == FR_PAGE) {	
		kprintf("free_frm %d %d %d\n", i, pid, frm_tab[i].fr_vpno);
		if(bsm_lookup(pid, frm_tab[i].fr_vpno, &store, &pageth) == SYSERR) {
			kprintf("free_frm: can't find map\n");
			restore(ps);
			return SYSERR;
		}
		
    	write_bs((char *)pt_e, store, pageth);
    	reset_frm(i);

		frm_tab[upper].fr_refcnt--;
	} else if(frm_tab[i].fr_type == FR_TBL) {
    	init_pt(pt_e);
    	reset_frm(i);

		frm_tab[upper].fr_refcnt--;
	} else {
  		init_pd(pd_e);
  		reset_frm(i);	
	}

	pdequeue(i);
	
	restore(ps);
  	return OK;
}


/*-------------------------------------------------------------------------
 * write_back_to_backing_store
 *-------------------------------------------------------------------------
 */
SYSCALL write_back_to_backing_store(int old_pid) {
	
	STATWORD ps;
	disable(ps);

	int i, upper, u_upper, store, pageth;

	pd_t *pd;	
	pt_t *pt;

	pd = proctab[old_pid].pdbr;

 	for(i = 0; i < NFRAMES; i++) {
 		if(frm_tab[i].fr_status == FRM_MAPPED && frm_tab[i].fr_type == FR_PAGE && frm_tab[i].fr_pid == old_pid) {
			pt = (pt_t *) fr2p(i);

			if( SYSERR == bsm_lookup(old_pid, frm_tab[i].fr_vpno, &store, &pageth)) {
				kprintf("write_back_to_backing_store: bsm_lookup can't find mapping with %d %d\n", old_pid, frm_tab[i].fr_vpno);
				kill(old_pid);
				restore(ps);
				return SYSERR;
			}
			
			kprintf("process <%d> writes frame %d to store %d with page offset %d (vaddr: %d)\n", old_pid, i, store, pageth, frm_tab[i].fr_vpno);

			write_bs((char *)pt, store, pageth);

			upper = frm_tab[i].fr_upper;
			if(--frm_tab[upper].fr_refcnt <= 0) {
				init_pt(pt);
				
				u_upper = frm_tab[upper].fr_upper;
	    		if(--frm_tab[u_upper].fr_refcnt <= 0) {
	    			init_pd(pd);
	    		}
			}
 		}
 	}
	
 	restore(ps);
    return OK;
}

/*-------------------------------------------------------------------------
 * read_from_backing_store
 *-------------------------------------------------------------------------
 */
SYSCALL read_from_backing_store(int new_pid) {
	STATWORD ps;
	disable(ps);

	int i, store, pageth;
	unsigned long vpno;

	pt_t *pt_e;

	for (i = 0; i < NSTORES; i++) {
		if (proctab[new_pid].bsmap[i].bs_status == BSM_MAPPED) {
			vpno = proctab[new_pid].bsmap[i].bs_vpno;
			if( SYSERR == bsm_lookup(new_pid, vpno, &store, &pageth)) {
				kprintf("read_from_backing_store: bsm_lookup can't find mapping with %d %d\n", new_pid, vpno);
				kill(new_pid);
				restore(ps);
				return SYSERR;
			}

			kprintf("process <%d> reads frame %d from store %d with page offset %d (vaddr: %d)\n", new_pid, i, store, pageth, vpno);

			pt_e = (pt_t*) vno2p(vpno);
			read_bs((char *)pt_e, store, pageth);
		}
	}
	
 	restore(ps);
    return OK;
}

/*------------------------------------------------------------------------
 * pnewqueue  --  initialize a new list in the pq structure
 *------------------------------------------------------------------------
 */
void pnewqueue()
{
	struct	qent	*hptr;
	struct	qent	*tptr;
	int	hindex, tindex;

	hptr = &pq[ hindex = NFRAMES]; /* assign and rememeber queue	*/
	tptr = &pq[ tindex = NFRAMES + 1]; /* index values for head&tail	*/
	hptr->qnext = tindex;
	hptr->qprev = tindex;
	tptr->qnext = hindex;
	tptr->qprev = hindex;
}

/*------------------------------------------------------------------------
 * penqueue  --	insert an item at the tail of a list
 *------------------------------------------------------------------------
 */
int penqueue(int item, int tail)
/*	int	item;			- item to enqueue on a list	*/
/*	int	tail;			- index in pq of list tail	*/
{
	struct	qent	*tptr;		/* points to tail entry		*/
	struct	qent	*mptr;		/* points to item entry		*/

	tptr = &pq[tail];
	mptr = &pq[item];
	mptr->qnext = tail;
	mptr->qprev = tptr->qprev;
	pq[tptr->qprev].qnext = item;
	tptr->qprev = item;
	return(item);
}


/*------------------------------------------------------------------------
 *  pdequeue  --  remove an item from the head of a list and return it
 *------------------------------------------------------------------------
 */
int pdequeue(int item)
{
	struct	qent	*mptr;		/* pointer to pq entry for item	*/

	mptr = &pq[item];
	if (mptr->qprev != NULL && mptr->qnext != NULL) {
		pq[mptr->qprev].qnext = mptr->qnext;
		pq[mptr->qnext].qprev = mptr->qprev;	
	}
	return(item);
}