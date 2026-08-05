.intel_syntax noprefix

.globl c_mm_alloc_pages
c_mm_alloc_pages:
        push ebp
        mov ebp, esp

        push ebx

        mov eax, 2
        mov ebx, [ebp + 8]
        mov ecx, [ebp + 12]
        int 0x80

        pop ebx

        mov esp, ebp
        pop ebp
        ret
