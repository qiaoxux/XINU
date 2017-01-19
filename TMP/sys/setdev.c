/* setdev.c - setdev */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lab0.h>

/*------------------------------------------------------------------------
 *  setdev  -  set the two device entries in the process table entry
 *------------------------------------------------------------------------
 */
SYSCALL	setdev(int pid, int dev1, int dev2)
{
	unsigned long start_time = ctr1000;
	if (sys_trace == TRUE) {
		sys_call_nums[currpid][13]++;
	}
	
	short	*nxtdev;

	if (isbadpid(pid))
		return(SYSERR);
	nxtdev = (short *) proctab[pid].pdevs;
	*nxtdev++ = dev1;
	*nxtdev = dev2;

	if (sys_trace == TRUE) {
		sys_call_time[currpid][13] += ctr1000 - start_time;
	}
	return(OK);
}
