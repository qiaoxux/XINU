/*	Task 3
-------------------------------------------------------------------------------------------------
For each existing process with larger priority than the parameter, print the stack base, stack size, stacklimit, and stack pointer. Also, for each process, include the process name, the process id and the process priority.

To help you do this, please look into proc.h in the h/ directory. Note the proctab[] array that holds all processes. Also, note that the pesp member of the pentry structure holds the saved stack pointer. Therefore, the currently executing process has a stack pointer that is different from the value of this variable. In order to help you get the stack pointer of the currently executing process, carefully study the stacktrace.c file in the sys/ directory. The register %esp holds the current stack pointer. You can use in-line assembly(i.e., asm("...")) to do this part.
-------------------------------------------------------------------------------------------------
*/

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab0.h>

unsigned long *espp;

void printprocstks(int priority) {
	kprintf("void printprocstks(int priority)\n\n");

	int i;
	struct pentry *proc;
	for (i = 0; i < NPROC; i++) {
		proc = &proctab[i];
		if (proc->pprio > priority && proc->pstate != PRFREE) {
			if (proc->pstate == PRCURR) {
				asm("movl %esp, espp");
			} else {
				espp = (unsigned long *)proc->pesp;
			}

			kprintf("Process [%s]\n", proc->pname);
	        kprintf("	pid: %d\n", i);
	        kprintf("	priority: %d\n", proc->pprio);
        	kprintf("	base: 0x%08x\n", proc->pbase);
	        kprintf("	limit: 0x%08x\n", proc->plimit);
	        kprintf("	len: %d\n", proc->pstklen);
        	kprintf("	pointer: 0x%08x\n", espp);
		}
	}
	
	kprintf("\n");
	return;
}