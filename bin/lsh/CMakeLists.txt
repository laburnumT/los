project(
  lsh
  LANGUAGES C
)

add_executable(
  lsh
  main.c
)

target_link_libraries(
  lsh
  PUBLIC
  lib_headers
  libc
  gcc
  ${CMAKE_CURRENT_SOURCE_DIR}/crti.s
  ${CRTBEGIN_O}
  ${CRTEND_O}
  ${CMAKE_CURRENT_SOURCE_DIR}/crtn.s
  ${CMAKE_CURRENT_SOURCE_DIR}/crt0.s
)

target_link_options(
  lsh
  PUBLIC
  -pie
  # -T ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
  -nostdlib
  -static
)

set_target_properties(
  lsh
  PROPERTIES
  OUTPUT_NAME lsh
)
