/*	Task 1
-------------------------------------------------------------------------------------------------
Print the addresses indicating the end of the text, data, and BSS segments of the Xinu OS. Also print the 4-byte contents (in hexadecimal) preceding and after those addresses. This function can be written in C.
-------------------------------------------------------------------------------------------------
*/

#include <stdlib.h>
#include <lab0.h>

extern char etext, edata, end;

void printsegaddress() {
	kprintf("void printsegaddress()\n\n");

	kprintf("Current: ");
    kprintf("etext[0x%08x]: 0x%08x, ", &etext - 1, *(&etext - 1));
    kprintf("edata[0x%08x]: 0x%08x, ", &edata - 1, *(&edata - 1));
    kprintf("ebss[0x%08x]: 0x%08x\n", &end - 1, *(&end - 1));

    kprintf("Preceding: ");
    kprintf("etext[0x%08x]: 0x%08x, ", &etext - 2, *(&etext - 2));
    kprintf("edata[0x%08x]: 0x%08x, ", &edata - 2, *(&edata - 2));
    kprintf("ebss[0x%08x]: 0x%08x\n", &end - 2, *(&end - 2));

    kprintf("After: ");
    kprintf("etext[0x%08x]: 0x%08x, ", &etext, etext);
    kprintf("edata[0x%08x]: 0x%08x, ", &edata, edata);
    kprintf("ebss[0x%08x]: 0x%08x\n\n", &end, end);

    return;
}