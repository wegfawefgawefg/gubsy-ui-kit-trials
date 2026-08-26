#include "RmlUi_Platform_SDL.h"
#include "RmlUi_Renderer_SDL_GPU.h"
#include "app.h"

#include <RmlUi/Core.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

struct Timings {
  double update_ms = 0.0;
  double render_record_ms = 0.0;
  double submit_ms = 0.0;
  double gpu_complete_ms = 0.0;
  double frame_ms = 0.0;
};

struct BenchmarkSamples {
  std::vector<double> update;
  std::vector<double> render_record;
  std::vector<double> submit;
  std::vector<double> gpu_complete;
  std::vector<double> frame;

  void Add(const Timings &value) {
    update.push_back(value.update_ms);
    render_record.push_back(value.render_record_ms);
    submit.push_back(value.submit_ms);
    gpu_complete.push_back(value.gpu_complete_ms);
    frame.push_back(value.frame_ms);
  }
};

struct Resolution {
  const char *label;
  int width;
  int height;
};

constexpr std::array<Resolution, 6> kResolutions{{
    {"1920 x 1080", 1920, 1080},
    {"1280 x 720", 1280, 720},
    {"Tablet 1024 x 768", 1024, 768},
    {"Phone landscape 915 x 412", 915, 412},
    {"Phone landscape 844 x 390", 844, 390},
    {"Phone portrait 412 x 915", 412, 915},
}};

constexpr std::array<const char *, 17> kTargetScreens{{
    "Play lobby",
    "Quest picker",
    "Session settings",
    "Session mods",
    "Players / Local",
    "Players / Profiles",
    "Players / Devices",
    "Settings / Display",
    "Settings / Audio",
    "Settings / Accessibility",
    "Settings / Gameplay",
    "Controls / Bindings",
    "Controls / Devices",
    "Controls / Input tuning",
    "Progress",
    "Mods / Installed",
    "Mods / Catalog",
}};
constexpr std::array<const char *, 4> kProviderStates{{
    "Populated",
    "Empty",
    "Loading",
    "Error",
}};

double milliseconds(Clock::time_point start, Clock::time_point end) {
  return std::chrono::duration<double, std::milli>(end - start).count();
}

long read_rss_kib() {
  std::ifstream status("/proc/self/status");
  std::string key;
  while (status >> key) {
    if (key == "VmRSS:") {
      long value = 0;
      status >> value;
      return value;
    }
    std::string ignored;
    std::getline(status, ignored);
  }
  return -1;
}

void print_metric(const char *name, std::vector<double> values) {
  if (values.empty())
    return;
  std::sort(values.begin(), values.end());
  const double mean = std::accumulate(values.begin(), values.end(), 0.0) /
                      static_cast<double>(values.size());
  const size_t p50_index = static_cast<size_t>((values.size() - 1) * 0.50);
  const size_t p95_index = static_cast<size_t>((values.size() - 1) * 0.95);
  const size_t p99_index = static_cast<size_t>((values.size() - 1) * 0.99);
  std::printf("  \"%s\": {\"mean_ms\": %.4f, \"p50_ms\": %.4f, \"p95_ms\": "
              "%.4f, \"p99_ms\": %.4f, \"max_ms\": %.4f}",
              name, mean, values[p50_index], values[p95_index],
              values[p99_index], values.back());
}

void print_help(const char *executable) {
  std::printf(
      "Usage: %s [--resolution WIDTHxHEIGHT] [--hidden] [--frames N]\n"
      "\n"
      "F1 toggles the native experiment panel. Escape exits.\n"
      "--screen N selects one of the 17 committed reference targets.\n"
      "--no-tools starts without the ImGui experiment panel.\n"
      "--benchmark N runs N uncapped frames and prints timing JSON.\n"
      "--capture FILE renders offscreen and writes a PNG without showing a "
      "window.\n"
      "--frames is useful for automated smoke runs under a display server.\n",
      executable);
}

bool parse_resolution(std::string_view value, int &width, int &height) {
  const size_t split = value.find('x');
  if (split == std::string_view::npos)
    return false;
  width = std::atoi(std::string(value.substr(0, split)).c_str());
  height = std::atoi(std::string(value.substr(split + 1)).c_str());
  return width > 0 && height > 0;
}

} // namespace

int main(int argc, char **argv) {
  int window_width = 1280;
  int window_height = 720;
  int frame_limit = 0;
  int initial_screen = 0;
  int initial_provider_state = 0;
  bool hidden = false;
  bool tools_visible_at_start = true;
  bool benchmark = false;
  bool self_test = false;
  std::string capture_path;

  for (int i = 1; i < argc; ++i) {
    const std::string_view argument(argv[i]);
    if (argument == "--help" || argument == "-h") {
      print_help(argv[0]);
      return 0;
    }
    if (argument == "--hidden") {
      hidden = true;
      continue;
    }
    if (argument == "--no-tools") {
      tools_visible_at_start = false;
      continue;
    }
    if (argument == "--benchmark" && i + 1 < argc) {
      benchmark = true;
      hidden = true;
      tools_visible_at_start = false;
      frame_limit = std::atoi(argv[++i]);
      if (frame_limit <= 120) {
        std::fprintf(stderr,
                     "Benchmark needs more than 120 frames for warmup.\n");
        return 2;
      }
      continue;
    }
    if (argument == "--capture" && i + 1 < argc) {
      capture_path = argv[++i];
      hidden = true;
      tools_visible_at_start = false;
      if (frame_limit == 0)
        frame_limit = 5;
      continue;
    }
    if (argument == "--self-test") {
      self_test = true;
      hidden = true;
      tools_visible_at_start = false;
      if (frame_limit == 0)
        frame_limit = 5;
      continue;
    }
    if (argument == "--screen" && i + 1 < argc) {
      initial_screen = std::atoi(argv[++i]);
      if (initial_screen < 0 ||
          initial_screen >= static_cast<int>(kTargetScreens.size())) {
        std::fprintf(stderr, "Invalid screen index. Expected 0 through 16.\n");
        return 2;
      }
      continue;
    }
    if (argument == "--provider" && i + 1 < argc) {
      initial_provider_state = std::atoi(argv[++i]);
      if (initial_provider_state < 0 || initial_provider_state >= 4) {
        std::fprintf(stderr, "Invalid provider state. Expected 0 through 3.\n");
        return 2;
      }
      continue;
    }
    if (argument == "--resolution" && i + 1 < argc) {
      if (!parse_resolution(argv[++i], window_width, window_height)) {
        std::fprintf(stderr, "Invalid resolution. Expected WIDTHxHEIGHT.\n");
        return 2;
      }
      continue;
    }
    if (argument == "--frames" && i + 1 < argc) {
      frame_limit = std::atoi(argv[++i]);
      continue;
    }
    std::fprintf(stderr, "Unknown argument: %s\n", argv[i]);
    return 2;
  }

  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
    std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
    return 1;
  }

  SDL_WindowFlags flags =
      SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_UTILITY;
  if (hidden)
    flags |= SDL_WINDOW_HIDDEN;
  SDL_Window *window = SDL_CreateWindow("Gubsy RmlUi Design Demo", window_width,
                                        window_height, flags);
  if (!window) {
    std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }
  const SDL_DisplayID primary_display = SDL_GetPrimaryDisplay();
  SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED_DISPLAY(primary_display),
                        SDL_WINDOWPOS_CENTERED_DISPLAY(primary_display));

  SDL_GPUDevice *gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV |
                                               SDL_GPU_SHADERFORMAT_DXIL |
                                               SDL_GPU_SHADERFORMAT_MSL,
                                           true, nullptr);
  if (!gpu || !SDL_ClaimWindowForGPUDevice(gpu, window)) {
    std::fprintf(stderr, "SDL GPU setup failed: %s\n", SDL_GetError());
    if (gpu)
      SDL_DestroyGPUDevice(gpu);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }
  SDL_SetGPUSwapchainParameters(gpu, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
                                benchmark ? SDL_GPU_PRESENTMODE_IMMEDIATE
                                          : SDL_GPU_PRESENTMODE_VSYNC);

  SystemInterface_SDL system_interface;
  system_interface.SetWindow(window);
  RenderInterface_SDL_GPU render_interface(gpu, window);
  Rml::SetSystemInterface(&system_interface);
  Rml::SetRenderInterface(&render_interface);
  if (!Rml::Initialise()) {
    std::fprintf(stderr, "RmlUi initialization failed.\n");
    return 1;
  }

  const std::string font_path =
      std::string(GUBSY_UI_BUILD_ASSET_DIR) + "/LatoLatin-Regular.ttf";
  if (!Rml::LoadFontFace(font_path))
    std::fprintf(stderr, "Could not load font: %s\n", font_path.c_str());
  const std::string bold_font_path =
      std::string(GUBSY_UI_BUILD_ASSET_DIR) + "/LatoLatin-Bold.ttf";
  if (!Rml::LoadFontFace(bold_font_path))
    std::fprintf(stderr, "Could not load font: %s\n", bold_font_path.c_str());
  const std::string fallback_font_path =
      std::string(GUBSY_UI_BUILD_ASSET_DIR) + "/DejaVuSans.ttf";
  if (!Rml::LoadFontFace(fallback_font_path, true))
    std::fprintf(stderr, "Could not load fallback font: %s\n",
                 fallback_font_path.c_str());

  int drawable_width = 0;
  int drawable_height = 0;
  SDL_GetWindowSizeInPixels(window, &drawable_width, &drawable_height);
  float display_density = SDL_GetWindowPixelDensity(window);
  int render_width =
      static_cast<int>(std::lround(drawable_width * display_density));
  int render_height =
      static_cast<int>(std::lround(drawable_height * display_density));
  SDL_GPUTexture *offscreen_texture = nullptr;
  SDL_GPUTransferBuffer *capture_transfer = nullptr;
  const bool use_offscreen = benchmark || !capture_path.empty();
  if (use_offscreen) {
    SDL_GPUTextureCreateInfo texture_info{};
    texture_info.type = SDL_GPU_TEXTURETYPE_2D;
    texture_info.format = SDL_GetGPUSwapchainTextureFormat(gpu, window);
    texture_info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    texture_info.width = render_width;
    texture_info.height = render_height;
    texture_info.layer_count_or_depth = 1;
    texture_info.num_levels = 1;
    texture_info.sample_count = SDL_GPU_SAMPLECOUNT_1;
    offscreen_texture = SDL_CreateGPUTexture(gpu, &texture_info);
    if (!offscreen_texture) {
      std::fprintf(stderr, "Could not create offscreen render target: %s\n",
                   SDL_GetError());
      return 1;
    }
    if (!capture_path.empty()) {
      SDL_GPUTransferBufferCreateInfo transfer_info{};
      transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
      transfer_info.size =
          static_cast<Uint32>(render_width * render_height * 4);
      capture_transfer = SDL_CreateGPUTransferBuffer(gpu, &transfer_info);
      if (!capture_transfer) {
        std::fprintf(stderr, "Could not create capture transfer buffer: %s\n",
                     SDL_GetError());
        return 1;
      }
    }
  }
  Rml::Context *context = Rml::CreateContext(
      "gubsy-shell", Rml::Vector2i(render_width, render_height));
  if (!context) {
    std::fprintf(stderr, "Could not create RmlUi context.\n");
    return 1;
  }
  // RmlUi lays out in logical game coordinates while its SDL_GPU geometry is
  // recorded into the high-density swapchain.
  context->SetDensityIndependentPixelRatio(display_density);
  const std::string document_path =
      std::string(GUBSY_UI_SOURCE_ASSET_DIR) + "/ui/shell.rml";
  const long rss_before_document = read_rss_kib();
  const auto document_start = Clock::now();
  auto app = std::make_unique<GubsyApp>(context);
  if (!app->Initialize(document_path)) {
    std::fprintf(stderr, "Could not load document: %s\n",
                 document_path.c_str());
    return 1;
  }
  app->SetViewport(drawable_width, drawable_height);
  app->SelectToolScreen(initial_screen);
  app->SetProviderState(initial_provider_state);
  app->Update();
  context->Update();
  if (self_test && !app->RunSelfTest()) {
    std::fprintf(stderr, "Native route/input self-test failed.\n");
    return 1;
  }
  const double document_build_ms = milliseconds(document_start, Clock::now());
  std::vector<double> hide_samples;
  std::vector<double> show_samples;
  if (benchmark) {
    hide_samples.reserve(100);
    show_samples.reserve(100);
    for (int sample = 0; sample < 100; ++sample) {
      auto transition_start = Clock::now();
      app->document()->Hide();
      context->Update();
      hide_samples.push_back(milliseconds(transition_start, Clock::now()));
      transition_start = Clock::now();
      app->document()->Show();
      context->Update();
      show_samples.push_back(milliseconds(transition_start, Clock::now()));
    }
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  ImGui::StyleColorsDark();
  ImGui_ImplSDL3_InitForSDLGPU(window);
  ImGui_ImplSDLGPU3_InitInfo imgui_info{};
  imgui_info.Device = gpu;
  imgui_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(gpu, window);
  imgui_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
  imgui_info.SwapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;
  imgui_info.PresentMode = SDL_GPU_PRESENTMODE_VSYNC;
  ImGui_ImplSDLGPU3_Init(&imgui_info);

  bool running = true;
  bool show_tools = tools_visible_at_start;
  int selected_resolution = 1;
  int selected_screen = initial_screen;
  int selected_provider_state = initial_provider_state;
  float physical_scale = display_density;
  float dp_ratio = 1.0f;
  int completed_frames = 0;
  Timings timings;
  BenchmarkSamples benchmark_samples;
  std::vector<SDL_Gamepad *> gamepads;
  int gamepad_count = 0;
  if (SDL_JoystickID *ids = SDL_GetGamepads(&gamepad_count)) {
    for (int i = 0; i < gamepad_count; ++i) {
      if (SDL_Gamepad *gamepad = SDL_OpenGamepad(ids[i]))
        gamepads.push_back(gamepad);
    }
    SDL_free(ids);
  }
  while (running) {
    const auto frame_start = Clock::now();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);
      if (event.type == SDL_EVENT_QUIT)
        running = false;
      if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
        if (SDL_Gamepad *gamepad = SDL_OpenGamepad(event.gdevice.which))
          gamepads.push_back(gamepad);
      }
      if (event.type == SDL_EVENT_GAMEPAD_REMOVED) {
        auto it = std::find_if(
            gamepads.begin(), gamepads.end(), [&](SDL_Gamepad *gamepad) {
              return SDL_GetGamepadID(gamepad) == event.gdevice.which;
            });
        if (it != gamepads.end()) {
          SDL_CloseGamepad(*it);
          gamepads.erase(it);
        }
      }
      if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_F1)
        show_tools = !show_tools;
      const bool app_consumed = app->HandleSdlEvent(event);
      if (!app_consumed)
        RmlSDL::InputEventHandler(context, window, event);
    }

    int next_width = 0;
    int next_height = 0;
    SDL_GetWindowSizeInPixels(window, &next_width, &next_height);
    const float next_density = SDL_GetWindowPixelDensity(window);
    const int next_render_width =
        static_cast<int>(std::lround(next_width * next_density));
    const int next_render_height =
        static_cast<int>(std::lround(next_height * next_density));
    if (next_width != drawable_width || next_height != drawable_height ||
        next_render_width != render_width ||
        next_render_height != render_height) {
      drawable_width = next_width;
      drawable_height = next_height;
      render_width = next_render_width;
      render_height = next_render_height;
      context->SetDimensions({render_width, render_height});
      app->SetViewport(drawable_width, drawable_height);
    }

    const auto update_start = Clock::now();
    app->Update();
    context->Update();
    const auto update_end = Clock::now();
    timings.update_ms = milliseconds(update_start, update_end);

    ImGui_ImplSDLGPU3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    if (show_tools) {
      ImGui::SetNextWindowPos({18.0f, 70.0f}, ImGuiCond_FirstUseEver);
      ImGui::SetNextWindowSize({360.0f, 0.0f}, ImGuiCond_FirstUseEver);
      ImGui::Begin("Native experiment controls", &show_tools,
                   ImGuiWindowFlags_AlwaysAutoResize);
      ImGui::TextUnformatted("F1 toggles this panel");
      if (ImGui::Combo("Target screen", &selected_screen, kTargetScreens.data(),
                       kTargetScreens.size()))
        app->SelectToolScreen(selected_screen);
      if (ImGui::Combo("Provider state", &selected_provider_state,
                       kProviderStates.data(), kProviderStates.size()))
        app->SetProviderState(selected_provider_state);
      if (ImGui::Combo(
              "Resolution", &selected_resolution,
              [](void *, int index) { return kResolutions[index].label; },
              nullptr, kResolutions.size())) {
        const Resolution &resolution = kResolutions[selected_resolution];
        SDL_SetWindowSize(window, resolution.width, resolution.height);
        SDL_SetWindowPosition(window,
                              SDL_WINDOWPOS_CENTERED_DISPLAY(primary_display),
                              SDL_WINDOWPOS_CENTERED_DISPLAY(primary_display));
      }
      if (ImGui::SliderFloat("RmlUi dp ratio", &dp_ratio, 0.5f, 3.0f, "%.2f"))
        context->SetDensityIndependentPixelRatio(physical_scale * dp_ratio);
      ImGui::Separator();
      ImGui::Text("Rml update: %.3f ms", timings.update_ms);
      ImGui::Text("Render record: %.3f ms", timings.render_record_ms);
      ImGui::Text("GPU submit: %.3f ms", timings.submit_ms);
      ImGui::Text("GPU complete wait: %.3f ms", timings.gpu_complete_ms);
      ImGui::Text("CPU frame: %.3f ms", timings.frame_ms);
      ImGui::Text("RSS: %ld KiB", read_rss_kib());
      ImGui::Text("GPU: %s", SDL_GetGPUDeviceDriver(gpu));
      ImGui::End();
    }
    ImGui::Render();

    SDL_GPUCommandBuffer *command_buffer = SDL_AcquireGPUCommandBuffer(gpu);
    if (!command_buffer) {
      std::fprintf(stderr, "Could not acquire command buffer: %s\n",
                   SDL_GetError());
      break;
    }
    SDL_GPUTexture *swapchain = nullptr;
    uint32_t swap_width = 0;
    uint32_t swap_height = 0;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(
            command_buffer, window, &swapchain, &swap_width, &swap_height)) {
      std::fprintf(stderr, "Could not acquire swapchain: %s\n", SDL_GetError());
      SDL_CancelGPUCommandBuffer(command_buffer);
      break;
    }
    if (swap_width > 0 && swap_height > 0 &&
        (static_cast<int>(swap_width) != render_width ||
         static_cast<int>(swap_height) != render_height)) {
      render_width = static_cast<int>(swap_width);
      render_height = static_cast<int>(swap_height);
      physical_scale = drawable_width > 0
                           ? static_cast<float>(render_width) / drawable_width
                           : 1.0f;
      context->SetDimensions({render_width, render_height});
      context->SetDensityIndependentPixelRatio(physical_scale * dp_ratio);
    }
    SDL_GPUTexture *frame_target = swapchain ? swapchain : offscreen_texture;
    const uint32_t frame_width =
        swapchain ? swap_width : static_cast<uint32_t>(render_width);
    const uint32_t frame_height =
        swapchain ? swap_height : static_cast<uint32_t>(render_height);
    if (frame_target) {
      SDL_GPUColorTargetInfo clear_target{};
      clear_target.texture = frame_target;
      clear_target.clear_color = {0.012f, 0.025f, 0.028f, 1.0f};
      clear_target.load_op = SDL_GPU_LOADOP_CLEAR;
      clear_target.store_op = SDL_GPU_STOREOP_STORE;
      SDL_GPURenderPass *clear_pass =
          SDL_BeginGPURenderPass(command_buffer, &clear_target, 1, nullptr);
      SDL_EndGPURenderPass(clear_pass);

      const auto render_start = Clock::now();
      render_interface.BeginFrame(command_buffer, frame_target, frame_width,
                                  frame_height);
      context->Render();
      render_interface.EndFrame();
      ImDrawData *draw_data = ImGui::GetDrawData();
      ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, command_buffer);
      SDL_GPUColorTargetInfo imgui_target{};
      imgui_target.texture = frame_target;
      imgui_target.load_op = SDL_GPU_LOADOP_LOAD;
      imgui_target.store_op = SDL_GPU_STOREOP_STORE;
      SDL_GPURenderPass *imgui_pass =
          SDL_BeginGPURenderPass(command_buffer, &imgui_target, 1, nullptr);
      ImGui_ImplSDLGPU3_RenderDrawData(draw_data, command_buffer, imgui_pass);
      SDL_EndGPURenderPass(imgui_pass);
      timings.render_record_ms = milliseconds(render_start, Clock::now());

      if (capture_transfer && frame_limit > 0 &&
          completed_frames + 1 >= frame_limit) {
        SDL_GPUTextureRegion source{};
        source.texture = frame_target;
        source.w = frame_width;
        source.h = frame_height;
        source.d = 1;
        SDL_GPUTextureTransferInfo destination{};
        destination.transfer_buffer = capture_transfer;
        destination.pixels_per_row = frame_width;
        destination.rows_per_layer = frame_height;
        SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(command_buffer);
        SDL_DownloadFromGPUTexture(copy_pass, &source, &destination);
        SDL_EndGPUCopyPass(copy_pass);
      }
    }

    const auto submit_start = Clock::now();
    SDL_SubmitGPUCommandBuffer(command_buffer);
    timings.submit_ms = milliseconds(submit_start, Clock::now());
    if (benchmark) {
      const auto gpu_wait_start = Clock::now();
      SDL_WaitForGPUIdle(gpu);
      timings.gpu_complete_ms = milliseconds(gpu_wait_start, Clock::now());
    }
    timings.frame_ms = milliseconds(frame_start, Clock::now());

    ++completed_frames;
    if (benchmark && completed_frames > 120)
      benchmark_samples.Add(timings);
    if (frame_limit > 0 && completed_frames >= frame_limit)
      running = false;
  }

  SDL_WaitForGPUIdle(gpu);
  const long rss_with_document = read_rss_kib();
  if (capture_transfer) {
    void *pixels = SDL_MapGPUTransferBuffer(gpu, capture_transfer, false);
    if (!pixels) {
      std::fprintf(stderr, "Could not map captured frame: %s\n",
                   SDL_GetError());
    } else {
      const SDL_GPUTextureFormat format =
          SDL_GetGPUSwapchainTextureFormat(gpu, window);
      const SDL_PixelFormat pixel_format =
          (format == SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM ||
           format == SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM_SRGB)
              ? SDL_PIXELFORMAT_BGRA32
              : SDL_PIXELFORMAT_RGBA32;
      SDL_Surface *surface = SDL_CreateSurfaceFrom(
          render_width, render_height, pixel_format, pixels, render_width * 4);
      if (!surface || !IMG_SavePNG(surface, capture_path.c_str()))
        std::fprintf(stderr, "Could not save capture '%s': %s\n",
                     capture_path.c_str(), SDL_GetError());
      if (surface)
        SDL_DestroySurface(surface);
      SDL_UnmapGPUTransferBuffer(gpu, capture_transfer);
    }
    SDL_ReleaseGPUTransferBuffer(gpu, capture_transfer);
  }
  if (offscreen_texture)
    SDL_ReleaseGPUTexture(gpu, offscreen_texture);
  ImGui_ImplSDLGPU3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
  const auto document_close_start = Clock::now();
  app.reset();
  const double document_close_ms =
      milliseconds(document_close_start, Clock::now());
  if (benchmark) {
    std::printf(
        "{\n  \"screen\": \"%s\",\n  \"viewport\": \"%dx%d\",\n  \"samples\": "
        "%zu,\n  \"ui_ready_ms\": %.4f,\n  \"document_close_ms\": %.4f,\n  "
        "\"process_rss_kib\": %ld,\n  \"document_rss_delta_kib\": %ld,\n",
        kTargetScreens[selected_screen], drawable_width, drawable_height,
        benchmark_samples.frame.size(), document_build_ms, document_close_ms,
        rss_with_document, rss_with_document - rss_before_document);
    print_metric("update", benchmark_samples.update);
    std::printf(",\n");
    print_metric("render_record", benchmark_samples.render_record);
    std::printf(",\n");
    print_metric("submit_cpu", benchmark_samples.submit);
    std::printf(",\n");
    print_metric("gpu_complete_wait", benchmark_samples.gpu_complete);
    std::printf(",\n");
    print_metric("cpu_frame", benchmark_samples.frame);
    std::printf(",\n");
    print_metric("resident_hide", hide_samples);
    std::printf(",\n");
    print_metric("resident_show", show_samples);
    std::printf("\n}\n");
  }
  for (SDL_Gamepad *gamepad : gamepads)
    SDL_CloseGamepad(gamepad);
  Rml::RemoveContext(context->GetName());
  Rml::Shutdown();
  render_interface.Shutdown();
  SDL_ReleaseWindowFromGPUDevice(gpu, window);
  SDL_DestroyGPUDevice(gpu);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
