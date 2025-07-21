project(lib)

add_library(
  libk
  stdio.c
  stdlib.c
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
  libc_headers
  INTERFACE
)

target_include_directories(
  libc_headers
  INTERFACE
  include
)
