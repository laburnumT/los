include_directories(include)

add_library(
  libelfk
  elf.c
)

target_compile_definitions(
  libelfk
  PRIVATE
  __is_libk
)

target_link_libraries(
  libelfk
  PUBLIC
  lib_headers
  libk
)

target_include_directories(
  libelfk
  INTERFACE
  include
)
