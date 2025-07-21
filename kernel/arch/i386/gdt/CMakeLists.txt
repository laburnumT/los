add_library(
  gdt
  gdt.c
  gdt_asm.s
)

target_link_libraries(
  gdt
  libc_headers
  i386
)
