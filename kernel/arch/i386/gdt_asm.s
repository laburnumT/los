.intel_syntax noprefix

.globl gdt_set, gdt_reload_segments

gdtr:
    .short 0
    .long 0

.type gdt_set, @function
gdt_set:
    mov  ax, [esp + 4]
    mov [gdtr], ax
    mov eax, [esp + 8]
    mov [gdtr + 2], eax
    lgdt [gdtr]
    ret

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
