add_library(
  syscall
  syscall.c
)

target_link_libraries(
  syscall
  PRIVATE
  kernel_headers
  lib_headers
  libelfk
)
