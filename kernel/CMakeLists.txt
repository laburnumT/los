add_compile_definitions(__is_kernel)

include_directories(include)

add_subdirectory(arch/i386)

add_executable(
  los.kernel
  kernel/keyboard.c
  kernel/kernel.c
  kernel/ps2_keyboard.c
)

target_link_options(
  los.kernel
  PRIVATE
  -T ${CMAKE_CURRENT_SOURCE_DIR}/arch/i386/linker.ld -nostdlib
)

target_link_libraries(
  los.kernel
  PUBLIC

  libk
  i386
  gcc
)

add_library(
  kernel_headers
  INTERFACE
)

target_include_directories(
  kernel_headers
  INTERFACE
  include
)
