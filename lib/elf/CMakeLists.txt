add_library(
  libelfk
  elf.c
)

target_link_libraries(
  libelfk
  PRIVATE
  lib_headers
  kernel_headers
)

target_include_directories(
  libelfk
  PUBLIC
  include
)
