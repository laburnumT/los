.intel_syntax noprefix

.globl _start
_start:
	call _init
	call main
	mov ebx, eax
	mov eax, 5
	int 0x80
