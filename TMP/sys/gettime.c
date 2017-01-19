/* gettime.c - gettime */

#include <conf.h>
#include <kernel.h>
#include <date.h>
#include <lab0.h>
#include <proc.h>

extern int getutim(unsigned long *);

/*------------------------------------------------------------------------
 *  gettime  -  get local time in seconds past Jan 1, 1970
 *------------------------------------------------------------------------
 */
SYSCALL	gettime(long *timvar)
{
    /* long	now; */

	/* FIXME -- no getutim */

	unsigned long start_time = ctr1000;
	if (sys_trace == TRUE) {
		sys_call_nums[currpid][4]++;
	}

	if (sys_trace == TRUE) {
		sys_call_time[currpid][4] += ctr1000 - start_time;
	}
    return OK;
}
