add_subdirectory(elf)
add_subdirectory(arch/i386)

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
  PUBLIC
  libk_i386
  libelfk
)

target_link_options(
  libk
  PRIVATE
)

target_include_directories(
  libk
  PUBLIC
  include
)

add_library(
  lib_headers
  INTERFACE
)

target_include_directories(
  lib_headers
  INTERFACE
  include
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
  PUBLIC
  libc_i386
)

target_link_options(
  libc
  PRIVATE
)

target_include_directories(
  libc
  PUBLIC
  include
)
