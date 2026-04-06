add_library(
  libk_i386
  k_stdio.c
)

target_link_libraries(
  libk_i386
  PUBLIC
  kernel_headers
)

target_compile_definitions(
  libk_i386
  PRIVATE
  __is_libk
)

add_library(
  libc_i386
  c_stdio.s
  c_stdio.c
)

target_compile_definitions(
  libc_i386
  PRIVATE
  __is_libc
)
