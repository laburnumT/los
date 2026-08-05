.intel_syntax noprefix

.globl c_putchar
c_putchar:
        push ebp
        mov ebp, esp

        push ebx

        mov eax, 0
        mov ebx, [ebp + 8]
        int 0x80

        pop ebx

        mov esp, ebp
        pop ebp
        ret

.globl c_getchar
c_getchar:
        push ebp
        mov ebp, esp

        push ebx

        mov eax, 1
        int 0x80

        pop ebx

        mov esp, ebp
        pop ebp
        ret
