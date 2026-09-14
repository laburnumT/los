project(
  lsh
  LANGUAGES C
)

add_executable(
  lsh
  main.c
)

target_include_directories(
  lsh
  PRIVATE
  include
)

target_link_libraries(
  lsh
  PRIVATE
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
  PRIVATE
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

add_custom_command(
  TARGET lsh POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${ROOTFS_DIR}/bin
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:lsh>
    ${ROOTFS_DIR}/bin/lsh
)
