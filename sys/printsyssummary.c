/*	Task 4
-------------------------------------------------------------------------------------------------
Print the summary of the system calls which have been invoked for each process. This task is loosely based on the functionality of LTTng . There are 43 system calls declared. Please look into kernel.h in the h/ directory to see all declared system calls. However, only 27 system calls are implemented in this XINU version. The implementation of these 27 system calls are in the sys/ directory. You are asked to print the frequency (how many times each system call type is invoked) and the average execution time (how long it takes to execute each system call type in average) of these 27 system calls for each process. In order to do this, you will need to modify the implementation of these 27 types of system calls to trace them whenever they are invoked. To measure the time, XINU provides a global variable named ctr1000 to track the time (in milliseconds) passed by since the system starts. Please look into sys/clkinit.c and sys/clkint.S to see the details. 

You will also need to implement two other functions: 
void syscallsummary_start(): to start tracing the system calls. All the system calls are invoked after calling this function (and before calling syscallsummary_stop()) will be presented in the system call summary.
void syscallsummary_stop(): to stop tracing the system calls. 
In other words, these two functions determine the duration in which the system calls are traced. 

To help you complete this task, we provide two files, syscalls.txt lists all the system calls you will need to trace, and test.c demonstrates the usage of the functions you will implement (note that this is only the test file and will not be used for grading).
-------------------------------------------------------------------------------------------------
*/

#include <conf.h>
#include <stdio.h>
#include <kernel.h>
#include <proc.h>
#include <lab0.h>

int sys_trace = FALSE;

int sys_call_time[NPROC][27];
int sys_call_nums[NPROC][27];
char sys_call_names[27][16] = {
	"sys_freemem",
	"sys_chprio",
	"sys_getpid",
	"sys_getprio",
	"sys_gettime",
	"sys_kill",
	"sys_receive",
	"sys_recvclr",
	"sys_recvtim",
	"sys_resume",
	"sys_scount",
	"sys_sdelete",
	"sys_send",
	"sys_setdev",
	"sys_setnok",
	"sys_screate",
	"sys_signal",
	"sys_signaln",
	"sys_sleep",
	"sys_sleep10",
	"sys_sleep100",
	"sys_sleep1000",
	"sys_sreset",
	"sys_stacktrace",
	"sys_suspend",
	"sys_unsleep",
	"sys_wait",
};

void syscallsummary_start() {
	sys_trace = TRUE;

	int i, j;
	for (i = 0; i < NPROC; i++) {
		for (j = 0; j < 27; j++) {
			sys_call_time[i][j] = 0;
			sys_call_nums[i][j] = 0;
		}
	}
}

void syscallsummary_stop() {
	sys_trace = FALSE;
}

void printsyssummary() {
	kprintf("void printsyscallsummary()\n\n");

	int i, j;
	struct pentry * proc;
	for (i = 0; i < NPROC; i++) {
		proc = &proctab[i];
		if (proc->pstate != PRFREE) {
			int title = FALSE;
			for (j = 0; j < 27; j++) {
				if (sys_call_nums[i][j] > 0) {
					if (title == FALSE) {
						kprintf("Process [pid:%d]\n", i);
						title = TRUE;
					}

					kprintf("	Syscall: %s, count: %d, average execution time: %d (ms)\n", \
						sys_call_names[j], sys_call_nums[i][j], sys_call_time[i][j]/sys_call_nums[i][j]);		
				}
			}
		}
	}
}

