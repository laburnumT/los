.intel_syntax noprefix

.globl gdt_reload_segments

gdt_reload_segments:
    jmp 0x08:.reload_segments
.reload_segments:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret
