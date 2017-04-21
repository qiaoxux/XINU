/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>


/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */

SYSCALL pfint() {
    STATWORD ps;
    disable(ps);

    virt_addr_t * vaddr;
  	int i, vp,free_frame,store,pageth;
  	unsigned int pd_offset, pt_offset, pg_offset;
  	unsigned long cr2, physical_addr;
    
    pd_t *pd;
    pt_t *pt;
    
    cr2 = read_cr2();
    vaddr = (virt_addr_t *)(&cr2); 
    vp = p2vno(cr2);
    pd = proctab[currpid].pdbr;

    if( SYSERR == bsm_lookup(currpid, vp, &store, &pageth)) {
      	kill(currpid);
      	return SYSERR;
    }

    pd_offset = vaddr->pd_offset;
    pt_offset = vaddr->pt_offset;
    pg_offset = vaddr->pg_offset;

    if(pd[pd_offset].pd_pres != 1) {
      get_frm(&free_frame);
      set_frm(free_frame, currpid, FR_TBL);

      frm_tab[free_frame].fr_upper = p2fr((unsigned long) pd);

      pt = fr2p(free_frame);
      init_pt(pt);

      pd[pd_offset].pd_pres = 1;
      pd[pd_offset].pd_write = 1;
      pd[pd_offset].pd_base = p2vno((unsigned long) pt);
    } else {
      frm_tab[pd[pd_offset].pd_base - 1024].fr_refcnt++;
    }

    pt = vno2p(pd[pd_offset].pd_base);
    
  	get_frm(&free_frame);
    set_frm(free_frame, currpid, FR_PAGE);
    frm_tab[free_frame].fr_bid = store;
    frm_tab[free_frame].fr_vpno = vp;
  	frm_tab[free_frame].fr_upper = pd[pd_offset].pd_base - FRAME0;
    
    if (frm_tab[free_frame].fr_age + 128 > 255) {
      frm_tab[free_frame].fr_age = 255;
    } else {
      frm_tab[free_frame].fr_age += 128;  
    }

  	pt[pt_offset].pt_pres  = 1;
    pt[pt_offset].pt_write = 1;
    pt[pt_offset].pt_base  = fr2vno(free_frame);

  	physical_addr = fr2p(free_frame);
  	read_bs(physical_addr, store, pageth);
  	// kprintf("pfint: %d %d %d %d %d\n", currpid, physical_addr, free_frame, store, pageth);

    penqueue(free_frame, TailPQ);
	
    set_PDBR(currpid);

    restore(ps);
    return OK;
}
