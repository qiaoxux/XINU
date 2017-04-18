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
SYSCALL init_frm()
{
	STATWORD ps;
  	disable(ps);

	int i;
	fr_map_t frm_tab[NFRAMES];

	for (i = 0; i < NFRAMES; i++) {
		frm_tab[i].fr_status = FRM_UNMAPPED;
		frm_tab[i].fr_pid = -1;
		frm_tab[i].fr_vpno = -1;
		frm_tab[i].fr_refcnt = 0;
		frm_tab[i].fr_type = FR_PAGE;
		frm_tab[i].fr_dirty = 0;
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
SYSCALL get_frm(int* avail)
{
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
			
			if (frm_tab[currqueue].fr_refcnt == 0) {
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

	kprintf("Wrong policy");
	return SYSERR;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
	STATWORD ps;
  	disable(ps);

	if (i < 0 || i > NFRAMES) {
		kprintf("Wrong frame index");
		return SYSERR;
	}

	int pid, pd_nframes;
	unsigned long vpno;
	unsigned long pdbr;
	unsigned int pd_offset, pt_offset;

	pd_t *pd_entry;
	pt_t *pt_entry;

	if (frm_tab[i].fr_status == FRM_MAPPED && frm_tab[i].fr_type == FR_PAGE) {

		vpno = frm_tab[i].fr_vpno;
		pdbr = proctab[(pid = frm_tab[i].fr_pid)].pdbr;
		
		pd_offset = vpno >> 10;
		pt_offset = vpno & 1023;
		pd_entry = pdbr + pd_offset * sizeof(pd_t);
		pt_entry = pd_entry->pd_base * NBPG + pt_offset * sizeof(pt_t);

		frm_tab[i].fr_status = FRM_UNMAPPED;
		frm_tab[i].fr_pid = -1;
		frm_tab[i].fr_vpno = -1;
		frm_tab[i].fr_refcnt = 0;
		frm_tab[i].fr_type = FR_PAGE;
		frm_tab[i].fr_dirty = 0;

		pt_entry->pt_pres = 0;

		write_bs((FRAME0 + i) * NBPG, proctab[pid].store, vpno - proctab[pid].vhpno);

		if(--frm_tab[pd_nframes].fr_refcnt <= 0) {
			pd_nframes = pd_entry->pd_base - FRAME0;

			frm_tab[pd_nframes].fr_status = FRM_UNMAPPED;
			frm_tab[pd_nframes].fr_pid = -1;
			frm_tab[pd_nframes].fr_vpno = -1;
			frm_tab[pd_nframes].fr_refcnt = 0;
			frm_tab[pd_nframes].fr_type = FR_PAGE;
			frm_tab[pd_nframes].fr_dirty = 0;
			
			pd_entry->pd_pres = 0;
		}
	}

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * evict_frm - evict frames belong to pid
 *-------------------------------------------------------------------------
 */
SYSCALL evict_frm(int pid)
{
	STATWORD ps;
  	disable(ps);

	if(isbadpid(pid)) {
		kprintf("Wrong process id");
		return SYSERR;
	}

	int i, qid;
	for (i = 0; i < NFRAMES; i++) {
		if (frm_tab[i].fr_status = FRM_MAPPED && frm_tab[i].fr_pid == pid && frm_tab[i].fr_type == FR_PAGE) {
			int qid = pq[HeadPQ].qnext;
			while (qid != TailPQ) {
				if (qid == i) {
					pdequeue(i);
					break;
				}
				qid = pq[qid].qnext;
			}
			free_frm(i);
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