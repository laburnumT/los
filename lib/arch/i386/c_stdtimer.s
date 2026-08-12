.intel_syntax noprefix

.globl c_sleep
c_sleep:
        push ebp
        mov ebp, esp

        push ebx

        mov eax, 3
        mov ebx, [ebp + 8]
        int 0x80

        pop ebx

        mov esp, ebp
        pop ebp
        ret
