/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>
#include <q.h>


/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
	STATWORD ps;
	disable(ps);

	int i, bs_id, offset, vpno, free_frame;
	struct idt *pidt;
	unsigned long vaddr, pdbr;
	unsigned int pd_offset, pt_offset;

	pt_t *pt_entry;
	pd_t *pd_entry;

	/* Contains a value called Page Fault Linear Address (PFLA). When a page fault occurs, 
	the address the program attempted to access is stored in the CR2 register. */
	vaddr = read_cr2();
	vpno = vaddr >> 12;
	pd_offset = vpno >> 10;
	pt_offset = vpno & 1023;

	pdbr = proctab[currpid].pdbr;
	pd_entry = pdbr + pd_offset * sizeof(pd_t);
	
	
	if (vpno > pd_offset * 1024 * 1024) {
		kprintf("Illegal address");
		kill(currpid);
		return SYSERR;
	}

	kprintf("currpid: %d\n", currpid);
	kprintf("vaddr: 0x%08x\n", vaddr);
	kprintf("vpno: %d\n", vpno);
	kprintf("pd_offset: %d\n", pd_offset);
	kprintf("pt_offset: %d\n", pt_offset);

	if (pd_entry->pd_pres == 0) {
		get_frm(&free_frame);

		pt_entry = (pt_t*)((FRAME0 + free_frame) * NBPG);
		for (i = 0; i < NFRAMES; i++) {
			pt_entry->pt_pres = 0;	
			pt_entry->pt_write = 0;
			pt_entry->pt_user = 0;
			pt_entry->pt_pwt = 0;
			pt_entry->pt_pcd = 0;
			pt_entry->pt_acc = 0;
			pt_entry->pt_dirty = 0;
			pt_entry->pt_mbz = 0;
			pt_entry->pt_global = 0;
			pt_entry->pt_avail = 0;
			pt_entry->pt_base = 0;	
			pt_entry++;
		}

		pd_entry->pd_pres = 1;	
		pd_entry->pd_write = 1;
		pd_entry->pd_user = 0;
		pd_entry->pd_pwt = 0;
		pd_entry->pd_pcd = 0;
		pd_entry->pd_acc = 0;
		pd_entry->pd_mbz = 0;
		pd_entry->pd_fmb = 0;
		pd_entry->pd_global = 0;
		pd_entry->pd_avail = 0;
		pd_entry->pd_base = FRAME0 + free_frame;

		frm_tab[free_frame].fr_status = FRM_UNMAPPED;
		frm_tab[free_frame].fr_pid = currpid;
		frm_tab[free_frame].fr_vpno = vpno;
		frm_tab[free_frame].fr_refcnt = 1;
		frm_tab[free_frame].fr_type = FR_TBL;
		frm_tab[free_frame].fr_dirty = 0;
	}

	pt_entry = (pt_t*)(pd_entry->pd_base * NBPG + pt_offset * sizeof(pt_t));

	bsm_lookup(currpid, vaddr, &bs_id, &offset);

	pt_entry->pt_pres = 1;	
	pt_entry->pt_write = 1;
	pt_entry->pt_user = 0;
	pt_entry->pt_pwt = 0;
	pt_entry->pt_pcd = 0;
	pt_entry->pt_acc = 0;
	pt_entry->pt_dirty = 0;
	pt_entry->pt_mbz = 0;
	pt_entry->pt_global = 0;
	pt_entry->pt_avail = 0;
	pt_entry->pt_base = 0;

	frm_tab[free_frame].fr_status = FRM_UNMAPPED;
	frm_tab[free_frame].fr_pid = currpid;
	frm_tab[free_frame].fr_vpno = vpno;
	frm_tab[free_frame].fr_refcnt++;
	frm_tab[free_frame].fr_type = FR_PAGE;
	frm_tab[free_frame].fr_dirty = 0;

	get_frm(&free_frame);

	read_bs((char*)((FRAME0 + free_frame) * NBPG), bs_id, offset);

	penqueue(free_frame, TailPQ);

	restore(ps);
	return OK;
}


