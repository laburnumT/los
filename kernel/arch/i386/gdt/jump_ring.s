.intel_syntax noprefix

.globl jump_ring

jump_ring:
    cli
    mov ax, (4 * 8) | 3
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov edx, DWORD PTR [esp + 0x8]
    mov eax, esp
    push (4 * 8) | 3
    push eax
    pushf
    push (3 * 8) | 3
    push edx
    iret
