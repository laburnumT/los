add_library(
  romfs
  romfs.c
)

target_link_libraries(
  romfs
  PRIVATE
  kernel_headers
  lib_headers
)
