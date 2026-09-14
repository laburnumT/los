add_subdirectory(vfs)
add_subdirectory(romfs)

add_library(
  fs
  INTERFACE
)


target_link_libraries(
  fs
  INTERFACE
  vfs
  romfs
)
