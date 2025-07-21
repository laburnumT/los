cmake_minimum_required(VERSION 3.20)

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
set(CMAKE_C_COMPILER i686-elf-gcc)

set(CMAKE_SYSROOT ${CMAKE_CURRENT_SOURCE_DIR}/sysroot)
file(
  MAKE_DIRECTORY
  ${CMAKE_SYSROOT}/usr/include
  ${CMAKE_SYSROOT}/usr/lib
  ${CMAKE_SYSROOT}/boot
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(
  los
  LANGUAGES C ASM
)

add_compile_options(
  -std=gnu23
  -g
  -Wall
  -Wextra
  -masm=intel
  -Wno-comment
  -ffreestanding
  -isystem=/usr/include
)

add_subdirectory(lib)
add_subdirectory(kernel)

target_link_libraries(
  los.kernel
  PUBLIC
  libc_headers
)

target_link_libraries(
  libk
  PUBLIC
  kernel_headers
)
