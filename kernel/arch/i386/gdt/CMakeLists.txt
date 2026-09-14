add_library(
  gdt
  gdt.c
  gdt_asm.s
  jump_ring.s
)

target_link_libraries(
  gdt
  PRIVATE
  kernel_headers
  lib_headers
  ${LOS_ARCH}
)
