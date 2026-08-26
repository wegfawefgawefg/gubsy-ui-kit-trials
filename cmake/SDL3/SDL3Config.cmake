# Adapter for machines where SDL3 is installed through pkg-config without a
# CMake package. SDL_image calls find_package(SDL3) in its own directory scope.
if(NOT TARGET SDL3::Headers)
  add_library(SDL3::Headers INTERFACE IMPORTED)
  set_property(TARGET SDL3::Headers PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    "${HOST_SDL3_INCLUDE_DIRS}")
endif()

if(NOT TARGET SDL3::SDL3)
  add_library(SDL3::SDL3 INTERFACE IMPORTED)
  set_property(TARGET SDL3::SDL3 PROPERTY INTERFACE_LINK_LIBRARIES
    PkgConfig::HOST_SDL3)
  set_property(TARGET SDL3::SDL3 PROPERTY SDL_FULL_VERSION
    "${HOST_SDL3_VERSION}")
endif()

set(SDL3_Headers_FOUND TRUE)
set(SDL3_SDL3_FOUND TRUE)
set(SDL3_FOUND TRUE)
