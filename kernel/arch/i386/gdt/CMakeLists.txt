add_library(
  gdt
  gdt.c
  gdt_asm.s
  jump_ring.s
)

target_link_libraries(
  gdt
  lib_headers
  ${LOS_ARCH}
)
