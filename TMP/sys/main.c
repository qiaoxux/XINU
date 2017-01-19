/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab0.h>

void halt();

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
int main()
{
	// Task 1
	long res = zfunction(0xaabbccdd);
	printf("\n%x\n\n", res);

	// Task 2
	printsegaddress();
	
	// Task 3
	printtos();
	
	// Task 4
	printprocstks(0);

	// Task 5
	syscallsummary_start();
   	sleep(10);
   	syscallsummary_stop();
   	printsyssummary();

	return 0;
}