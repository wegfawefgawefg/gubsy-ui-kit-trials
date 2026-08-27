cmake_minimum_required(VERSION 3.25)

project(gubsy_rmlui_design_demo VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Use the host SDL3 exactly as Gubsy/Splonks will. Other dependencies are pinned
# so performance comparisons remain reproducible.
find_package(PkgConfig REQUIRED)
pkg_check_modules(HOST_SDL3 REQUIRED IMPORTED_TARGET GLOBAL sdl3>=3.2.0)
set(SDL3_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SDL3" CACHE PATH "" FORCE)

set(SDLIMAGE_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_TESTS OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_VENDORED OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_GIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_JXL OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_LBM OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_PCX OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_PNM OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_QOI OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_SVG OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_TIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_WEBP OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_XCF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_XPM OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_XV OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  SDL3_image
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
  GIT_TAG d78dd65454f442bfdbc373201165f1ef579484ab # release-3.2.6
  GIT_SHALLOW TRUE
  GIT_SUBMODULES ""
)
FetchContent_MakeAvailable(SDL3_image)

set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(RMLUI_SAMPLES OFF CACHE BOOL "" FORCE)
set(RMLUI_LUA_BINDINGS OFF CACHE BOOL "" FORCE)
set(RMLUI_LOTTIE_PLUGIN OFF CACHE BOOL "" FORCE)
set(RMLUI_SVG_PLUGIN OFF CACHE BOOL "" FORCE)
set(RMLUI_FONT_ENGINE freetype CACHE STRING "" FORCE)
FetchContent_Declare(
  RmlUi
  GIT_REPOSITORY https://github.com/mikke89/RmlUi.git
  GIT_TAG 2230d1a6e8e0848ed87a5761e2a5160b2a175ba4 # 6.2
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(RmlUi)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG 9a5d5c45f54b1301ea471622eddede70384243af # v1.92.4
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)

add_library(imgui_sdlgpu STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlgpu3.cpp
)
target_include_directories(imgui_sdlgpu PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui_sdlgpu PUBLIC PkgConfig::HOST_SDL3)

# We instantiate RmlUi's platform and renderer interfaces ourselves. This lets
# RmlUi, the game compositor, and the ImGui experiment controls share one SDL
# window, GPU device, swapchain, and command buffer.
add_library(rmlui_sdlgpu_backend STATIC
  ${rmlui_SOURCE_DIR}/Backends/RmlUi_Platform_SDL.cpp
  ${rmlui_SOURCE_DIR}/Backends/RmlUi_Renderer_SDL_GPU.cpp
)
target_include_directories(rmlui_sdlgpu_backend PUBLIC ${rmlui_SOURCE_DIR}/Backends)
target_compile_definitions(rmlui_sdlgpu_backend PUBLIC RMLUI_SDL_VERSION_MAJOR=3)
target_link_libraries(rmlui_sdlgpu_backend PUBLIC
  RmlUi::RmlUi
  PkgConfig::HOST_SDL3
  SDL3_image-static
)

add_executable(gubsy-rmlui-demo
  src/main.cpp
  src/app_actions.cpp
  src/app_input.cpp
  src/app_lifecycle.cpp
  src/app_navigation.cpp
  src/app_navigation_self_test.cpp
  src/app_render.cpp
  src/app_self_test.cpp
  src/benchmark.cpp
  src/focus_elements.cpp
  src/gamepads.cpp
  src/native_assets.cpp
  src/screen_controls.cpp
  src/screen_mods.cpp
  src/screen_play.cpp
  src/screen_players.cpp
  src/screen_progress.cpp
  src/screen_router.cpp
  src/screen_settings.cpp
  src/ui_markup.cpp
  src/trial_options.cpp
)
target_link_libraries(gubsy-rmlui-demo PRIVATE
  RmlUi::RmlUi
  rmlui_sdlgpu_backend
  imgui_sdlgpu
)
target_compile_definitions(gubsy-rmlui-demo PRIVATE
  GUBSY_UI_SOURCE_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
  GUBSY_UI_BUILD_ASSET_DIR="${CMAKE_CURRENT_BINARY_DIR}/assets"
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/assets)
configure_file(
  ${rmlui_SOURCE_DIR}/Samples/assets/LatoLatin-Regular.ttf
  ${CMAKE_CURRENT_BINARY_DIR}/assets/LatoLatin-Regular.ttf
  COPYONLY
)
configure_file(
  ${rmlui_SOURCE_DIR}/Samples/assets/LatoLatin-Bold.ttf
  ${CMAKE_CURRENT_BINARY_DIR}/assets/LatoLatin-Bold.ttf
  COPYONLY
)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/assets/DejaVuSans.ttf
  ${CMAKE_CURRENT_BINARY_DIR}/assets/DejaVuSans.ttf
  COPYONLY
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(gubsy-rmlui-demo PRIVATE -Wall -Wextra -Wpedantic)
endif()
