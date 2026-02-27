add_library(
  syscall
  syscall.c
)

target_link_libraries(
  syscall
  lib_headers
)
