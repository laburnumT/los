cmake_minimum_required(VERSION 3.20)

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
set(ROOTFS_DIR "${CMAKE_BINARY_DIR}/rootfs")

file(
  MAKE_DIRECTORY
  ${ROOTFS_DIR}
)

if("${CMAKE_BUILD_TYPE}" STREQUAL "OPTIMISED")
  add_compile_options(-O2)
else()
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

project(
  los
  LANGUAGES C ASM
)

add_compile_options(
  -std=gnu23
  -ggdb3
  -Wall
  -Wextra
  -masm=intel
  -Wno-comment
  -ffreestanding
  -isystem=/usr/include
)

add_subdirectory(lib)
add_subdirectory(bin)
add_subdirectory(kernel)

target_link_libraries(
  los.kernel
  PRIVATE
  lib_headers
)

add_custom_target(
  copy_scripts
  ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/scripts
  ${CMAKE_CURRENT_BINARY_DIR}/scripts
)
