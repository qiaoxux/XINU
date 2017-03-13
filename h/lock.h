/* lock.h - readers/writer lock */

#ifndef _LOCK_H_
#define _LOCK_H_


#ifndef	NLOCKS
#define	NLOCKS	50	/* number of locks, if not defined	*/
#endif

#define DELETED 	-6	/* waiting on a deleted lock will return DELETED */

#define	LFREE	'\01'		/* this lock is free		*/
#define	LUSED	'\02'		/* this lock is used		*/

#define READ 	-1	/* reader lock */
#define WRITE 	1	/* writer lock */

struct	lentry	{		/* lock table entry		*/
	char 	lstate;		/* the state LFREE or LUSED		*/
	int	ltype;		/* the type READ or WRITE		*/
	int	lockcnt;		/* count for this lock		*/
	int lprio;		/* A priority field indicating the maximum priority among all the 
					processes waiting in the lock's wait queue.		*/
	int lprocs[NPROC];		/* to record process ids of the processes currently holding the lock. 	*/
	int	lqhead;		/* q index of head of list		*/
	int	lqtail;		/* q index of tail of list		*/
};
extern	struct	lentry	locktab[];
extern	int	nextlock;

#define	isbadlock(l) (l<0 || l>=NLOCKS)

extern int lcreate(void);
extern int ldelete(int lockdescriptor);
extern int lock(int ldes1, int type, int priority);
extern int releaseall(int numlocks, long args, ...);

#endif