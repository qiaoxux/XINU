/*	Task 2
-------------------------------------------------------------------------------------------------
Print the address of the top of the run-time stack for whichever process you are currently in, right before and right after you get into the printos() function call. In addition, print the contents of upto four stack locations below the top of the stack (the four or fewer items that have been the most recently pushed, if any). Remember that stack elements are 32 bits wide, and be careful to perform pointer arithmetic correctly. Also note that there are local variables and arguments on the stack, among other things. See the hints given for #4 below, especially on stacktrace.c and proc.h. Your function can be written entirely in C, or you can use in-line assembly if you prefer.
-------------------------------------------------------------------------------------------------
*/

#include <stdio.h>
#include <lab0.h>

unsigned long *espp, *ebpp;

void printtos() {
	kprintf("void printtos()\n\n");

	asm("movl %ebp, ebpp");

	kprintf("Before[0x%08x]: 0x%08x\n", ebpp + 2, *(ebpp + 2));
    kprintf("After [0x%08x]: 0x%08x\n", ebpp, *ebpp);

	int var1 = 0xaaaaaaaa;
    int var2 = 0xbbbbbbbb;
    int var3 = 0xcccccccc;
    int var4 = var1 + var2;

    asm("movl %esp, espp");

	int i;
	for (i = 0; i < 4; i++) {
		printf("	element[0x%08x]: 0x%08x\n", espp + i, *(espp + i));
	}

	kprintf("\n");
	return;
}