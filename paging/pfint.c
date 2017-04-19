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

	int i, bs_id, vpno, offset, free_frame;
	struct idt *pidt;
	virt_addr_t *virt_address;
	unsigned long vaddr, pdbr;
	unsigned int pg_offset, pd_offset, pt_offset;

	pt_t *pt_entry;
	pd_t *pd_entry;

	/* Contains a value called Page Fault Linear Address (PFLA). When a page fault occurs, 
	the address the program attempted to access is stored in the CR2 register. */
	vaddr = read_cr2();

	virt_address = (virt_addr_t*)&vaddr;

	pg_offset = virt_address->pg_offset;
	pt_offset = virt_address->pt_offset;
	pd_offset = virt_address->pd_offset;

	vpno = vaddr >> 12;
	if (vpno < 4096) {
		kprintf("Illegal address\n");
		kill(currpid);
		return SYSERR;
	}

	pdbr = proctab[currpid].pdbr;
	pd_entry = (pd_t*)(pdbr + pd_offset * sizeof(pd_t));
	if (pd_entry->pd_pres == 0) {
		get_frm(&free_frame);

		pt_entry = (pt_t*)((FRAME0 + free_frame) * NBPG);
		for (i = 0; i < NFRAMES; i++) {
			pt_entry->pt_pres = 0;	
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

		frm_tab[free_frame].fr_status = FRM_MAPPED;
		frm_tab[free_frame].fr_pid = currpid;
		frm_tab[free_frame].fr_vpno = vpno;
		frm_tab[free_frame].fr_refcnt = 1;
		frm_tab[free_frame].fr_type = FR_TBL;
		frm_tab[free_frame].fr_dirty = 0;
	}

	// kprintf("currpid: %d\n", currpid);
		
	// kprintf("vaddr: 0x%08x\n", vaddr);
	// kprintf("pg_offset: %d\n", pg_offset);
	// kprintf("pt_offset: %d\n", pt_offset);
	// kprintf("pd_offset: %d\n", pd_offset);

	pt_entry = (pt_t*)(pd_entry->pd_base * NBPG + pt_offset * sizeof(pt_t));

	if (pt_entry->pt_pres == 0) {
		get_frm(&free_frame);

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
		pt_entry->pt_base = FRAME0 + free_frame;

		frm_tab[free_frame].fr_status = FRM_MAPPED;
		frm_tab[free_frame].fr_pid = currpid;
		frm_tab[free_frame].fr_vpno = vpno;
		frm_tab[free_frame].fr_refcnt++;
		frm_tab[free_frame].fr_type = FR_PAGE;
		frm_tab[free_frame].fr_dirty = 0;

		bsm_lookup(currpid, vaddr, &bs_id, &offset);
		read_bs((char*)((FRAME0 + free_frame) * NBPG), bs_id, offset);
		penqueue(free_frame, TailPQ);
	}

	restore(ps);
	return OK;
}


