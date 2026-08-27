cmake_minimum_required(VERSION 3.25)
project(gubsy_raygui_trial VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED IMPORTED_TARGET raylib>=5.0)
find_package(X11 REQUIRED)

FetchContent_Declare(
  raygui
  GIT_REPOSITORY https://github.com/raysan5/raygui.git
  GIT_TAG 5.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raygui)

add_executable(gubsy-raygui-trial src/main.cpp src/x11_utility.cpp)
target_include_directories(gubsy-raygui-trial PRIVATE ${raygui_SOURCE_DIR}/src)
target_link_libraries(gubsy-raygui-trial PRIVATE PkgConfig::RAYLIB X11::X11)
target_compile_options(gubsy-raygui-trial PRIVATE -Wall -Wextra -Wpedantic)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/../vue-reference/public/DejaVuSans.ttf
  ${CMAKE_CURRENT_BINARY_DIR}/DejaVuSans.ttf
  COPYONLY
)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/../vue-reference/public/mod-sheet-1.png
  ${CMAKE_CURRENT_BINARY_DIR}/mod-sheet-1.png
  COPYONLY
)
target_compile_definitions(gubsy-raygui-trial PRIVATE
  GUBSY_RAYGUI_FONT_PATH="${CMAKE_CURRENT_BINARY_DIR}/DejaVuSans.ttf"
  GUBSY_RAYGUI_MOD_SHEET_PATH="${CMAKE_CURRENT_BINARY_DIR}/mod-sheet-1.png"
)
