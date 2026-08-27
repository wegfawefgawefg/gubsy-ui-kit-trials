cmake_minimum_required(VERSION 3.25)

project(gubsy_arbor_trial VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

set(ARBOR_BUILD_APP ON CACHE BOOL "" FORCE)
set(ARBOR_BUILD_SHOWCASE OFF CACHE BOOL "" FORCE)
set(ARBOR_FETCH_DEPENDENCIES ON CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

set(ARBOR_LOCAL_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/../../arbor-ui"
    CACHE PATH "Optional local Arbor checkout")
if(EXISTS "${ARBOR_LOCAL_SOURCE}/CMakeLists.txt")
  add_subdirectory("${ARBOR_LOCAL_SOURCE}" "${CMAKE_BINARY_DIR}/_deps/arbor-build")
  set(GUBSY_ARB_FONT_PATH "${ARBOR_LOCAL_SOURCE}/assets/fonts/InterVariable.ttf")
else()
  FetchContent_Declare(
    Arbor
    GIT_REPOSITORY https://github.com/f0ursqu4r3/arbor-ui.git
    GIT_TAG 3ee3749f3b7bd827bbea4092c1513cb41bbffc78
    GIT_SHALLOW TRUE
  )
  FetchContent_MakeAvailable(Arbor)

  # Arbor main at the pinned revision omits its text dependency from the
  # public widget link interface. Keep the trial reproducible until upstream
  # carries the one-line correction already present in the local checkout.
  target_link_libraries(arbor_widgets PUBLIC arbor_text)
  set(GUBSY_ARB_FONT_PATH "${arbor_SOURCE_DIR}/assets/fonts/InterVariable.ttf")
endif()

add_executable(gubsy-arbor-trial
  src/benchmark.cpp
  src/controller_input.cpp
  src/main.cpp
  src/trial_app.cpp
  src/trial_host.cpp
  src/trial_options.cpp
)
target_link_libraries(gubsy-arbor-trial PRIVATE arbor_app arbor_sdl3_gpu)
target_compile_definitions(gubsy-arbor-trial PRIVATE
  GUBSY_ARB_UI_DIR="${CMAKE_CURRENT_SOURCE_DIR}/ui"
  GUBSY_ARB_FONT="${GUBSY_ARB_FONT_PATH}"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(gubsy-arbor-trial PRIVATE -Wall -Wextra -Wpedantic)
endif()
