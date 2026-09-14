add_library(
  lib_headers
  INTERFACE
)

target_include_directories(
  lib_headers
  INTERFACE
  include
)

add_subdirectory(elf)
add_subdirectory(arch/${LOS_ARCH})

add_library(
  libk
  stdio.c
  stdlib.c
  stdlib_memory.c
  stdtimer.c
  string.c
)

target_compile_definitions(
  libk
  PRIVATE
  __is_libk
)

target_link_libraries(
  libk
  PRIVATE
  lib_headers
  kernel_headers
  libk_${LOS_ARCH}
  libelfk
)

add_library(
  libc
  stdio.c
  stdlib.c
  stdlib_memory.c
  stdtimer.c
  string.c
)

target_compile_definitions(
  libc
  PRIVATE
  __is_libc
)

target_link_libraries(
  libc
  PRIVATE
  lib_headers
  libc_${LOS_ARCH}
)
