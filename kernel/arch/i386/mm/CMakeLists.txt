add_library(
  mm

  mm.c
)

target_link_libraries(
  mm
  libc_headers
  i386
)
