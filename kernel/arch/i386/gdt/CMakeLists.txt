add_library(
  gdt
  gdt.c
  gdt_asm.s
  jump_ring.s
)

target_link_libraries(
  gdt
  libc_headers
  i386
)
