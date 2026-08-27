cmake_minimum_required(VERSION 3.25)
project(gubsy_nuklear_trial VERSION 0.1.0 LANGUAGES C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2>=2.0.20)
find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)

FetchContent_Declare(nuklear GIT_REPOSITORY https://github.com/Immediate-Mode-UI/Nuklear.git
  GIT_TAG a53ad2c658151071501372a5e0e5e978153835aa GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(nuklear)

add_executable(gubsy-nuklear-trial src/main.c)
target_include_directories(gubsy-nuklear-trial PRIVATE
  ${nuklear_SOURCE_DIR} ${nuklear_SOURCE_DIR}/demo/sdl_opengl3)
target_link_libraries(gubsy-nuklear-trial PRIVATE PkgConfig::SDL2 OpenGL::GL X11::X11 m)
target_compile_definitions(gubsy-nuklear-trial PRIVATE
  GUBSY_NUKLEAR_FONT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../vue-reference/public/DejaVuSans.ttf")
target_compile_options(gubsy-nuklear-trial PRIVATE -Wall -Wextra -Wpedantic)
