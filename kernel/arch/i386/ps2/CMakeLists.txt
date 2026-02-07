add_library(
  ps2
  ps2.c
  ps2_keyboard.c
  ps2_mouse.c
)

target_link_libraries(
  ps2
  libc_headers
  i386
)
