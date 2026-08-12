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
add_subdirectory(ps2)
add_subdirectory(syscall)

add_library(
  i386
  boot.s
  init.c
  panic.c
  pit.c
  port.c
  timer.c
  tty.c
)

target_link_libraries(
  i386
  PUBLIC
  libk
  lib_headers
  gdt
  interrupt
  mm
  ps2
  syscall
  ${CMAKE_CURRENT_SOURCE_DIR}/crti.s
  ${CRTBEGIN_O}
  ${CRTEND_O}
  ${CMAKE_CURRENT_SOURCE_DIR}/crtn.s
)
