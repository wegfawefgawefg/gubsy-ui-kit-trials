cmake_minimum_required(VERSION 3.25)
project(gubsy_dear_imgui_trial VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED IMPORTED_TARGET GLOBAL sdl3>=3.2.0)

FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG 9a5d5c45f54b1301ea471622eddede70384243af GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(imgui)

add_library(imgui_sdlgpu STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlgpu3.cpp)
target_include_directories(imgui_sdlgpu PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui_sdlgpu PUBLIC PkgConfig::SDL3)

add_executable(gubsy-dear-imgui-trial src/main.cpp src/ui.cpp)
target_link_libraries(gubsy-dear-imgui-trial PRIVATE imgui_sdlgpu PkgConfig::SDL3)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../vue-reference/public/DejaVuSans.ttf
  ${CMAKE_CURRENT_BINARY_DIR}/DejaVuSans.ttf COPYONLY)
target_compile_definitions(gubsy-dear-imgui-trial PRIVATE
  GUBSY_IMGUI_FONT_PATH="${CMAKE_CURRENT_BINARY_DIR}/DejaVuSans.ttf")
target_compile_options(gubsy-dear-imgui-trial PRIVATE -Wall -Wextra -Wpedantic)
