execute_process(
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen_interrupt_asm.py
  OUTPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/interrupt_asm_auto.s
)

add_library(
  interrupt

  idt.c
  interrupt.c
  interrupt_asm.s
  interrupt_asm_auto.s
)

target_link_libraries(
  interrupt
  lib_headers
  ${LOS_ARCH}
)
