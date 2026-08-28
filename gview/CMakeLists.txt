cmake_minimum_required(VERSION 3.25)
project(gubsy_gview_trial VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
find_package(PkgConfig REQUIRED)
pkg_check_modules(HOST_SDL3 REQUIRED IMPORTED_TARGET GLOBAL sdl3>=3.2.0)
set(SDL3_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../rmlui/cmake/SDL3" CACHE PATH "" FORCE)

FetchContent_Declare(
  trial_imgui_source
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.9b
  GIT_SHALLOW TRUE)
FetchContent_GetProperties(trial_imgui_source)
if(NOT trial_imgui_source_POPULATED)
  if(POLICY CMP0169)
    cmake_policy(PUSH)
    cmake_policy(SET CMP0169 OLD)
  endif()
  FetchContent_Populate(trial_imgui_source)
  if(POLICY CMP0169)
    cmake_policy(POP)
  endif()
endif()
add_library(trial_imgui STATIC
  "${trial_imgui_source_SOURCE_DIR}/imgui.cpp"
  "${trial_imgui_source_SOURCE_DIR}/imgui_draw.cpp"
  "${trial_imgui_source_SOURCE_DIR}/imgui_tables.cpp"
  "${trial_imgui_source_SOURCE_DIR}/imgui_widgets.cpp"
  "${trial_imgui_source_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp"
  "${trial_imgui_source_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp")
target_include_directories(trial_imgui PUBLIC
  "${trial_imgui_source_SOURCE_DIR}"
  "${trial_imgui_source_SOURCE_DIR}/backends")
target_link_libraries(trial_imgui PUBLIC PkgConfig::HOST_SDL3)

set(GVIEW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GVIEW_WITH_SDL3_RENDERER ON CACHE BOOL "" FORCE)
set(GVIEW_WITH_IMGUI ON CACHE BOOL "" FORCE)
set(GVIEW_IMGUI_TARGET trial_imgui CACHE STRING "" FORCE)
set(GVIEW_GLAYOUT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../glayout" CACHE PATH "" FORCE)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../gview" "${CMAKE_CURRENT_BINARY_DIR}/_deps/gview")

set(SDLIMAGE_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_TESTS OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_VENDORED OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_GIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_JXL OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_WEBP OFF CACHE BOOL "" FORCE)
FetchContent_Declare(SDL3_image
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
  GIT_TAG d78dd65454f442bfdbc373201165f1ef579484ab
  GIT_SHALLOW TRUE
  GIT_SUBMODULES "")
FetchContent_MakeAvailable(SDL3_image)

add_executable(gubsy-gview-trial
  src/app.cpp
  src/input.cpp
  src/main.cpp
  src/model.cpp
  src/screen_controls.cpp
  src/screen_game_ui.cpp
  src/screen_mods.cpp
  src/screen_play.cpp
  src/screen_players.cpp
  src/screen_progress.cpp
  src/screen_settings.cpp
  src/self_test.cpp
  src/shell.cpp
  src/view_builder.cpp)
target_link_libraries(gubsy-gview-trial PRIVATE gview::sdl3 gview::imgui trial_imgui SDL3_image-static)
target_compile_definitions(gubsy-gview-trial PRIVATE
  GVIEW_TRIAL_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../rmlui/assets"
  GVIEW_TRIAL_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(gubsy-gview-trial PRIVATE
  -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)
