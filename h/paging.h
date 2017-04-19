/* paging.h */

typedef unsigned int	 bsd_t;

/* Structure for a page directory entry */

typedef struct {
  unsigned int pd_pres	: 1;		/* page table present?		*/
  unsigned int pd_write : 1;		/* page is writable?		*/
  unsigned int pd_user	: 1;		/* is use level protection?	*/
  unsigned int pd_pwt	: 1;		/* write through cachine for pt?*/
  unsigned int pd_pcd	: 1;		/* cache disable for this pt?	*/
  unsigned int pd_acc	: 1;		/* page table was accessed?	*/
  unsigned int pd_mbz	: 1;		/* must be zero			*/
  unsigned int pd_fmb	: 1;		/* four MB pages?		*/
  unsigned int pd_global: 1;		/* global (ignored)		*/
  unsigned int pd_avail : 3;		/* for programmer's use		*/
  unsigned int pd_base	: 20;		/* location of page table?	*/
} pd_t;

/* Structure for a page table entry */

typedef struct {

  unsigned int pt_pres	: 1;		/* page is present?		*/
  unsigned int pt_write : 1;		/* page is writable?		*/
  unsigned int pt_user	: 1;		/* is use level protection?	*/
  unsigned int pt_pwt	: 1;		/* write through for this page? */
  unsigned int pt_pcd	: 1;		/* cache disable for this page? */
  unsigned int pt_acc	: 1;		/* page was accessed?		*/
  unsigned int pt_dirty : 1;		/* page was written?		*/
  unsigned int pt_mbz	: 1;		/* must be zero			*/
  unsigned int pt_global: 1;		/* should be zero in 586	*/
  unsigned int pt_avail : 3;		/* for programmer's use		*/
  unsigned int pt_base	: 20;		/* location of page?		*/
} pt_t;

typedef struct {
  unsigned int pg_offset : 12;		/* page offset			*/
  unsigned int pt_offset : 10;		/* page table offset		*/
  unsigned int pd_offset : 10;		/* page directory offset	*/
} virt_addr_t;

typedef struct frame {
  int fr_status;      /* MAPPED or UNMAPPED   */
  int fr_pid;       /* process id using this frame  */
  int fr_vpno;        /* corresponding virtual page no*/
  int fr_refcnt;      /* reference count    */
  int fr_type;        /* FR_DIR, FR_TBL, FR_PAGE  */
  int fr_dirty;

  int fr_id;  /* frame id */
  int fr_upper; /* page -> page table, page table -> page directory */
  struct frame *fr_next  /* the list of all frames on the same backing store */
} fr_map_t;

typedef struct bs {
  int bs_status;			/* MAPPED or UNMAPPED		*/
  int bs_pid;				/* process id using this slot   */
  int bs_vpno;				/* starting virtual page number */
  int bs_npages;			/* number of pages in the store */
  int bs_sem;				/* semaphore mechanism ?	*/

  int bs_private;        /* created by vcreate or not */
  int bs_nmapping;    /* how many mappings on this bs */
  fr_map_t  *bs_frames;  /* the list of frames on this bs */
} bs_map_t;

extern bs_map_t bsm_tab[];
extern fr_map_t frm_tab[];

/* Prototypes for required API calls */
SYSCALL xmmap(int, bsd_t, int);
SYSCALL xunmap(int);
SYSCALL vcreate(int *procaddr, int ssize, int hsize, int priority, char *name, int nargs, long args);
WORD *vgetmem(unsigned nbytes);
SYSCALL srpolicy (int policy);
SYSCALL vfreemem(struct mblock *block, unsigned size);

/* given calls for dealing with backing store */
int get_bs(bsd_t, unsigned int);
SYSCALL release_bs(bsd_t);
SYSCALL read_bs(char *, bsd_t, int);
SYSCALL write_bs(char *, bsd_t, int);

SYSCALL init_bsm();
SYSCALL init_bsmap_for_process(bs_map_t *);
SYSCALL get_bsm(int *);
SYSCALL free_bsm(int);
SYSCALL bsm_lookup(int, long, int *, int *);
SYSCALL bsm_map(int, int, int, int);
SYSCALL bsm_unmap(int, int, int);

SYSCALL init_frm();
SYSCALL get_frm(int *);
SYSCALL init_frm_after_get(int, int, int);
SYSCALL reset_frm(int);
SYSCALL free_frm(int);
SYSCALL find_frm(int, int);
SYSCALL decrease_frm_refcnt(int, int);
SYSCALL write_back(int);
SYSCALL read_from(int);

SYSCALL init_4_global_page_tables();
SYSCALL init_page_directory_for_process(int);
SYSCALL init_pt(pt_t *);
SYSCALL init_pd(pd_t *);

#define NBPG		4096	/* number of bytes per page	*/
#define FRAME0		1024	/* zero-th frame		*/
#define NFRAMES 	1024	/* number of frames		*/
#define NSTORES   8     /* number of stores   */

#define BSM_UNMAPPED	0
#define BSM_MAPPED	1

#define FRM_UNMAPPED	0
#define FRM_MAPPED	1

#define FR_PAGE		0
#define FR_TBL		1
#define FR_DIR		2

#define SC 3
#define AGING 4

#define BACKING_STORE_BASE	0x00800000
#define BACKING_STORE_UNIT_SIZE 0x00100000

#define fr2vno(i)  ( (unsigned int) (FRAME0 + i) )
#define fr2p(i)    ( (unsigned int) ((FRAME0 + i) * NBPG) )
#define p2fr(i)    ( (unsigned int) ((i/NBPG) - FRAME0) )
#define bs2p(i)    ( (unsigned int) ((2048 + i * 256) * NBPG) )
#define vno2p(i)   ( (unsigned int) (i * NBPG) )
#define p2vno(i)   ( (unsigned int) (i/NBPG) )
