add_library(
  vfs
  vfs.c
)

target_link_libraries(
  vfs
  PRIVATE
  kernel_headers
  lib_headers
)
