/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>
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

		frm_tab[i].fr_id = i;
		frm_tab[i].fr_next = NULL;
		frm_tab[i].fr_upper = -1;
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

	if (grpolicy() == SC) {
		while (1) {
			if (currqueue == HeadPQ || currqueue == TailPQ)
				currqueue = pq[HeadPQ].qnext;
			
			if (frm_tab[currqueue].fr_refcnt <= 0) {
				pdequeue(currqueue);
				free_frm(currqueue);
				*avail = currqueue;

				restore(ps);
				return OK;
			} else {
				frm_tab[currqueue].fr_refcnt--;
			}

			currqueue = pq[currqueue].qnext;
		}
	} else {
		int min_age = 256;
		int frame_idx = pq[TailPQ].qprev, fit_frame;
		while (frame_idx != HeadPQ) {
			frm_tab[frame_idx].fr_refcnt = (int) (frm_tab[frame_idx].fr_refcnt/2);
			frame_idx = pq[frame_idx].qprev;
		}

		frame_idx = pq[TailPQ].qprev;
		fit_frame = frame_idx;
		while (frame_idx != HeadPQ) {
			if (frm_tab[frame_idx].fr_refcnt <= min_age) {
				min_age = frm_tab[frame_idx].fr_refcnt;
				fit_frame = frame_idx;
			}
			frame_idx = pq[frame_idx].qprev;
		}

		pdequeue(fit_frame);
		free_frm(fit_frame);
		*avail = fit_frame;
		
		restore(ps);
		return OK;
	}

	return SYSERR;
}

/*-------------------------------------------------------------------------
 * init_frm_after_get - initialize a frame after get_frm
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm_after_get(int i, int pid, int type) {
	STATWORD ps;
    disable(ps);

	frm_tab[i].fr_status = FRM_MAPPED;
	frm_tab[i].fr_pid = pid;
	frm_tab[i].fr_refcnt = 1;
	frm_tab[i].fr_type = type;
	frm_tab[i].fr_dirty = 0;

	frm_tab[i].fr_id = i;
	frm_tab[i].fr_next = NULL;
	frm_tab[i].fr_upper = -1;

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

	frm_tab[i].fr_id = i;
	frm_tab[i].fr_next = NULL;
	frm_tab[i].fr_upper = -1;

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i) {
	STATWORD ps;
    disable(ps);

    int upper, store, pageth;

	pt_t *pt;
	pd_t *pd;
	
	pt = fr2p(i);
	
	upper = frm_tab[i].fr_upper;
	
	if(frm_tab[i].fr_type == FR_PAGE) {	
		if(bsm_lookup(currpid, frm_tab[i].fr_vpno, &store, &pageth) == SYSERR)
			kill(currpid);
 
    	write_bs((char *)pt, store, pageth);
    	init_pt(pt);
    	reset_frm(i);

		if(--frm_tab[upper].fr_refcnt <= 0)
			free_frm(upper);
	
	} else if(frm_tab[i].fr_type == FR_TBL) {
    	init_pt(pt);
    	reset_frm(i);

		if(--frm_tab[upper].fr_refcnt <= 0)
			free_frm(upper);
	} else {
		pd = fr2p(i);
  		init_pd(pd);
  		reset_frm(i);	
	}
	
	restore(ps);
  	return OK;
}

/*-------------------------------------------------------------------------
 * find_frm - find a frame using bs_id and bs_pageth
 *-------------------------------------------------------------------------
 */
SYSCALL find_frm(int pid, int vpno) {
	STATWORD ps;
    disable(ps);

    int i;
    for (i = 0; i < NFRAMES; i++) {
        if (frm_tab[i].fr_status == FRM_MAPPED && frm_tab[i].fr_type == FR_PAGE && 
        	frm_tab[i].fr_pid == pid && frm_tab[i].fr_vpno == vpno) {
            restore(ps);
            return i;
        }

    }

    restore(ps);
    return -1;
}

/*-------------------------------------------------------------------------
 * dec_frm_refcnt - decrease the reference count for each frame allocated
 * to this process for one particular backing store.
 *-------------------------------------------------------------------------
 */
SYSCALL decrease_frm_refcnt(int pid, int store) {
	STATWORD ps;
    disable(ps);

 	fr_map_t *curr, *prev;
 	curr = proctab[pid].bsmap[store].bs_frames;

 	while(curr != NULL) {
 		curr->fr_refcnt--;
 		if(curr->fr_refcnt <= 0) {
 			prev = curr;
 			free_frm(prev->fr_id);
 		}
 		curr = curr->fr_next;
 	}

 	restore(ps);
 	return OK;
}

/*-------------------------------------------------------------------------
 * write_back
 *-------------------------------------------------------------------------
 */
SYSCALL write_back(int old_pid) {
	
	STATWORD ps;
	disable(ps);

	int i, j = 0, upper, u_upper, store, pageth;
	unsigned int pd_offset,pt_offset,pg_offset;

	pd_t *pd;	
	pt_t *pt;

	pd = proctab[old_pid].pdbr;

 	for(i = 0; i < NFRAMES; i++) {
 		if(frm_tab[i].fr_status == FRM_MAPPED && frm_tab[i].fr_type == FR_PAGE && frm_tab[i].fr_pid == old_pid) {
			pt = (pt_t *) fr2p(i);
			upper = frm_tab[i].fr_upper;
		
			if( SYSERR == bsm_lookup(old_pid, frm_tab[i].fr_vpno, &store, &pageth)) {
				kprintf("write_back: bsm_lookup can't find mapping\n");
				kill(old_pid);
			}
			
			kprintf("write_back %d %d %d %d\n", old_pid, frm_tab[i].fr_vpno, store, pageth);

			write_bs((char *)pt, store, pageth);

	    	init_pt(pt);
	    	reset_frm(i);
			
			if(--frm_tab[upper].fr_refcnt <= 0) {	
				u_upper = frm_tab[upper].fr_upper;
	    		
	    		if(--frm_tab[u_upper].fr_refcnt <= 0)
	    			init_pd(pd);
			}

 		}
 	}
	
 	restore(ps);
    return OK;
}

/*-------------------------------------------------------------------------
 * read_from
 *-------------------------------------------------------------------------
 */
SYSCALL read_from(int new_pid) {
	
	STATWORD ps;
	disable(ps);

	int i, j = 0, upper, u_upper, store, pageth;
	unsigned int pd_offset,pt_offset,pg_offset;

	pd_t *pd;	
	pt_t *pt;

	pd = proctab[new_pid].pdbr;

	kprintf("%d find you\n", new_pid);

 	for(i = 0; i < NFRAMES; i++) {
 		if(frm_tab[i].fr_status == FRM_MAPPED && frm_tab[i].fr_type == FR_PAGE && frm_tab[i].fr_pid == new_pid) {
			pt = (pt_t *) fr2p(i);
			upper = frm_tab[i].fr_upper;
		
			if( SYSERR == bsm_lookup(new_pid, frm_tab[i].fr_vpno, &store, &pageth)) {
				kill(new_pid);
				kprintf("read_from: bsm_lookup can't find mapping\n");
			}
			
			kprintf("read_from %d %d %d %d\n", new_pid, frm_tab[i].fr_vpno, store, pageth);

			read_bs((char *)pt, store, pageth);
			
			frm_tab[upper].fr_refcnt++;

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
	pq[mptr->qprev].qnext = mptr->qnext;
	pq[mptr->qnext].qprev = mptr->qprev;
	return(item);
}