add_compile_definitions(__is_kernel)

add_library(
  kernel_headers
  INTERFACE
)

target_include_directories(
  kernel_headers
  INTERFACE
  include
)

add_subdirectory(arch/${LOS_ARCH})

add_executable(
  los.kernel
  kernel/kernel.c
  kernel/keyboard.c
)

target_link_options(
  los.kernel
  PRIVATE
  -T ${CMAKE_CURRENT_SOURCE_DIR}/arch/${LOS_ARCH}/linker.ld
  -nostdlib
)

target_link_libraries(
  los.kernel
  PRIVATE

  kernel_headers
  "-Wl,--start-group"
  libk
  libelfk
  ${LOS_ARCH}
  gcc
  "-Wl,--end-group"
)
