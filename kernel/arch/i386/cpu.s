.intel_syntax noprefix

.globl get_eflags, get_cr3

get_eflags:
    pushfd
    pop eax
    push eax
    popfd
    ret

get_cr3:
    mov eax, cr3
    ret
