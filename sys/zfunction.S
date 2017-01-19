.globl zfunction
zfunction:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	andl	$-4186113, %eax
	sall	$4, %eax
	leave
	ret