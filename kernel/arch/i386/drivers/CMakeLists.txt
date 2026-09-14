add_subdirectory(ps2)

add_library(
  ${LOS_ARCH}_drivers

  pic.c
  pit.c
  port.c
  tty.c
)

target_link_libraries(
  ${LOS_ARCH}_drivers
  PRIVATE
  ps2
  libk
)
