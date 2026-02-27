add_library(
  libk
  stdio.c
  stdlib.c
  stdlib_memory.c
  string.c
)

target_compile_definitions(
  libk
  PRIVATE
  __is_libk
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
