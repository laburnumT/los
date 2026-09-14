add_library(
  mm
  mm.c
)

target_link_libraries(
  mm
  PRIVATE
  kernel_headers
  lib_headers
  ${LOS_ARCH}
)
