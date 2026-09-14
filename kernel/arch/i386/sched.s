.intel_syntax noprefix

.globl sched_arch_switch_context_helper
sched_arch_switch_context_helper:
    mov esp, [esp + 4]
    pop ds
    popa
    add esp, 8
    iret
