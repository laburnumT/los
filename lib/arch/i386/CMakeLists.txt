add_library(
  libk_${LOS_ARCH}
  k_stdio.c
)

target_link_libraries(
  libk_${LOS_ARCH}
  PRIVATE
  kernel_headers
)

target_compile_definitions(
  libk_${LOS_ARCH}
  PRIVATE
  __is_libk
)

add_library(
  libc_${LOS_ARCH}
  c_stdio.s
  c_stdio.c
  c_stdlib.s
  c_stdtimer.s

  c_sched.c
)

target_link_libraries(
  libc_${LOS_ARCH}
  PRIVATE
  lib_headers
)

target_compile_definitions(
  libc_${LOS_ARCH}
  PRIVATE
  __is_libc
)
