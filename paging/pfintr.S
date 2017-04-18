/* pfintr.S - pfintr */


    	   .text
pferrcode: .long 0
           .globl  pfintr,pferrcode 
pfintr:
		popl pferrcode
		cli
		pushal
		call pfint
		popal
		sti
		iret