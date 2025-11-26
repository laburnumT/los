add_library(
  memory

  mm.c
)

target_link_libraries(
  memory
  libc_headers
  i386
)
