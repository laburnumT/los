include_directories(include)

execute_process(
  COMMAND ${CMAKE_C_COMPILER} -print-file-name=crtbegin.o
  OUTPUT_VARIABLE CRTBEGIN_O
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${CMAKE_C_COMPILER} -print-file-name=crtend.o
  OUTPUT_VARIABLE CRTEND_O
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_subdirectory(gdt)
add_subdirectory(interrupt)
add_subdirectory(mm)
add_subdirectory(syscall)
add_subdirectory(drivers)

add_library(
  ${LOS_ARCH}
  boot.s
  cpu.s
  init.c
  panic.c
  sched.c
  sched.s
  timer.c
)

target_link_libraries(
  ${LOS_ARCH}
  PRIVATE
  ${LOS_ARCH}_drivers
  kernel_headers
  lib_headers
  gdt
  interrupt
  mm
  syscall
  libelfk
  ${CMAKE_CURRENT_SOURCE_DIR}/crti.s
  ${CRTBEGIN_O}
  ${CRTEND_O}
  ${CMAKE_CURRENT_SOURCE_DIR}/crtn.s
)
