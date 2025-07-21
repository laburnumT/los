add_library(
  interrupt

  idt.c
  interrupt.c
  interrupt_asm.s
  interrupt_asm_auto.s
  pic.c
)

target_link_libraries(
  interrupt
  libc_headers
  i386
)
