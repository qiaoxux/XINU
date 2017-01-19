/* scount.c - scount */

#include <conf.h>
#include <kernel.h>
#include <sem.h>
#include <lab0.h>
#include <proc.h>

/*------------------------------------------------------------------------
 *  scount  --  return a semaphore count
 *------------------------------------------------------------------------
 */
SYSCALL scount(int sem)
{
	unsigned long start_time = ctr1000;
	if (sys_trace == TRUE) {
		sys_call_nums[currpid][10]++;
	}

	extern	struct	sentry	semaph[];

	if (isbadsem(sem) || semaph[sem].sstate==SFREE)
		return(SYSERR);

	if (sys_trace == TRUE) {
		sys_call_time[currpid][10] += ctr1000 - start_time;
	}
	return(semaph[sem].semcnt);
}
