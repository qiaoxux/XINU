#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>


/*-------------------------------------------------------------------------
 * init_4_global_page_tables
 *-------------------------------------------------------------------------
 */
SYSCALL init_4_global_page_tables() {
	STATWORD ps;
	disable(ps);

	int i, j, free_frame;
	for (i = 0; i < 4; i++) {
		get_frm(&free_frame);
		init_frm_after_get(free_frame, NULLPROC, FR_TBL);

		pt_t *new_pt = fr2p(free_frame);
		for (j = 0; j < 1024; ++j) {
			new_pt[j].pt_pres = 1;
			new_pt[j].pt_write = 1;
			new_pt[j].pt_user = 0;
			new_pt[j].pt_pwt = 0;
			new_pt[j].pt_pcd = 0;
			new_pt[j].pt_acc = 0;
			new_pt[j].pt_dirty = 0;
			new_pt[j].pt_mbz = 0;
			new_pt[j].pt_global = 1;
			new_pt[j].pt_avail = 0;
			new_pt[j].pt_base = i * 1024 + j;
		}

	}

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * init_page_directory_for_process
 *-------------------------------------------------------------------------
 */
SYSCALL init_page_directory_for_process(int pid) {
	STATWORD ps;
    disable(ps);

	int i, free_frame;
	get_frm(&free_frame);
	init_frm_after_get(free_frame, pid, FR_DIR);
	
	pd_t *new_pd = fr2p(free_frame);
	init_pd(new_pd);

	for(i = 0; i < 4; i++) {
		new_pd[i].pd_pres = 1;
		new_pd[i].pd_write = 1;
		new_pd[i].pd_user = 0;
		new_pd[i].pd_pwt = 0;
		new_pd[i].pd_pcd = 0;
		new_pd[i].pd_acc = 0;
		new_pd[i].pd_mbz = 0;
		new_pd[i].pd_fmb = 0;
		new_pd[i].pd_global = 1;
		new_pd[i].pd_avail = 0;
		new_pd[i].pd_base = FRAME0 + i;
	}
	
	proctab[pid].pdbr = (unsigned long) new_pd;
	
	init_bsmap_for_process(pid);
	
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * init_pd
 *-------------------------------------------------------------------------
 */
SYSCALL init_pd(pd_t *new_pd) {
	STATWORD ps;
    disable(ps);

    int i;
	for (i = 4; i < 1024; i++) {
		new_pd[i].pd_pres = 0;
		new_pd[i].pd_write = 1;
		new_pd[i].pd_user = 0;
		new_pd[i].pd_pwt = 0;
		new_pd[i].pd_pcd = 0;
		new_pd[i].pd_acc = 0;
		new_pd[i].pd_mbz = 0;
		new_pd[i].pd_fmb = 0;
		new_pd[i].pd_global = 0;
		new_pd[i].pd_avail = 0;
		new_pd[i].pd_base = 0;	
	}

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * init_pt
 *-------------------------------------------------------------------------
 */
SYSCALL init_pt(pt_t *new_pt) {
	STATWORD ps;
    disable(ps);

    int i;
	for (i = 0; i < 1024; i++) {
		new_pt[i].pt_pres = 0;
		new_pt[i].pt_write = 1;
		new_pt[i].pt_user = 0;
		new_pt[i].pt_pwt = 0;
		new_pt[i].pt_pcd = 0;
		new_pt[i].pt_acc = 0;
		new_pt[i].pt_dirty = 0;
		new_pt[i].pt_mbz = 0;
		new_pt[i].pt_global = 0;
		new_pt[i].pt_avail = 0;
		new_pt[i].pt_base = 0;	
	}

	restore(ps);
	return OK;
}
