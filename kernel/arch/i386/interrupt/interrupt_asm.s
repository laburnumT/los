.intel_syntax noprefix

.globl interrupt_enable
interrupt_enable:
	sti
	ret

.globl interrupt_disable
interrupt_disable:
	cli
	ret

.globl interrupt_exception_handler_wrapper
interrupt_exception_handler_wrapper:
	cli

	pushad

	xor eax, eax
	mov ax, ds
	push eax

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	push esp
	cld
	call interrupt_exception_handler
	pop eax

	pop eax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	popad
	add esp, 8

	iret
