add_library(
  mm

  mm.c
)

target_link_libraries(
  mm
  lib_headers
  ${LOS_ARCH}
)
