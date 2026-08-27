#include "RmlUi_Platform_SDL.h"
#include "RmlUi_Renderer_SDL_GPU.h"
#include "app.h"
#include "benchmark.h"
#include "gamepads.h"
#include "native_assets.h"
#include "trial_options.h"

#include <RmlUi/Core.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

int main(int argc, char **argv) {
  TrialOptions options;
  const ParseResult parse_result = parse_trial_options(argc, argv, options);
  if (parse_result != ParseResult::Run)
    return parse_result == ParseResult::ExitSuccess ? 0 : 2;

  const int window_width = options.window_width;
  const int window_height = options.window_height;
  const int frame_limit = options.frame_limit;
  const int initial_screen = options.initial_screen;
  const int initial_provider_state = options.initial_provider_state;
  const bool hidden = options.hidden;
  const bool benchmark = options.benchmark;
  const bool self_test = options.self_test;
  const std::string &capture_path = options.capture_path;

  // init SDL and window
  SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
    std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
    return 1;
  }
  SDL_SetGamepadEventsEnabled(true);

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

  // init GPU device
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

  // init RmlUi backend
  SystemInterface_SDL system_interface;
  system_interface.SetWindow(window);
  RenderInterface_SDL_GPU render_interface(gpu, window);
  Rml::SetSystemInterface(&system_interface);
  Rml::SetRenderInterface(&render_interface);
  if (!Rml::Initialise()) {
    std::fprintf(stderr, "RmlUi initialization failed.\n");
    return 1;
  }

  load_trial_fonts();

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
  // init hidden render target
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
  // load retained document
  Rml::Context *context = Rml::CreateContext(
      "gubsy-shell", Rml::Vector2i(render_width, render_height));
  if (!context) {
    std::fprintf(stderr, "Could not create RmlUi context.\n");
    return 1;
  }
  // map logical layout into the high-density swapchain
  context->SetDensityIndependentPixelRatio(display_density);
  const std::string document_path =
      std::string(GUBSY_UI_SOURCE_ASSET_DIR) + "/ui/shell.rml";
  const long rss_before_document = read_rss_kib();
  const auto document_start = TrialClock::now();
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
  const double document_build_ms =
      milliseconds(document_start, TrialClock::now());
  std::vector<double> hide_samples;
  std::vector<double> show_samples;
  if (benchmark) {
    hide_samples.reserve(100);
    show_samples.reserve(100);
    for (int sample = 0; sample < 100; ++sample) {
      auto transition_start = TrialClock::now();
      app->document()->Hide();
      context->Update();
      hide_samples.push_back(
          milliseconds(transition_start, TrialClock::now()));
      transition_start = TrialClock::now();
      app->document()->Show();
      context->Update();
      show_samples.push_back(
          milliseconds(transition_start, TrialClock::now()));
    }
  }

  // init dev tools
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
  // init frame state
  bool show_tools = options.tools_visible;
  int selected_resolution = 1;
  int selected_screen = initial_screen;
  int selected_provider_state = initial_provider_state;
  float physical_scale = display_density;
  float dp_ratio = 1.0f;
  int completed_frames = 0;
  Timings timings;
  BenchmarkSamples benchmark_samples;
  OpenGamepads gamepads;
  open_connected_gamepads(gamepads, *app);

  while (running) {
    const auto frame_start = TrialClock::now();
    // pump input
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);
      if (event.type == SDL_EVENT_QUIT)
        running = false;
      handle_gamepad_connection(event, gamepads, *app);
      if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_F1)
        show_tools = !show_tools;
      const bool app_consumed = app->HandleSdlEvent(event);
      if (!app_consumed)
        RmlSDL::InputEventHandler(context, window, event);
    }

    // match drawable size
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

    // update retained document
    const auto update_start = TrialClock::now();
    app->Update();
    context->Update();
    const auto update_end = TrialClock::now();
    timings.update_ms = milliseconds(update_start, update_end);

    // draw dev tools
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

    // draw shared GPU frame
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

      const auto render_start = TrialClock::now();
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
      timings.render_record_ms =
          milliseconds(render_start, TrialClock::now());

      // capture final frame
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

    // submit frame
    const auto submit_start = TrialClock::now();
    SDL_SubmitGPUCommandBuffer(command_buffer);
    timings.submit_ms = milliseconds(submit_start, TrialClock::now());
    if (benchmark) {
      const auto gpu_wait_start = TrialClock::now();
      SDL_WaitForGPUIdle(gpu);
      timings.gpu_complete_ms =
          milliseconds(gpu_wait_start, TrialClock::now());
    }
    timings.frame_ms = milliseconds(frame_start, TrialClock::now());

    ++completed_frames;
    if (benchmark && completed_frames > 120)
      benchmark_samples.Add(timings);
    if (frame_limit > 0 && completed_frames >= frame_limit)
      running = false;
  }

  // report measurements and free resources
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
  const auto document_close_start = TrialClock::now();
  app.reset();
  const double document_close_ms =
      milliseconds(document_close_start, TrialClock::now());
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
  close_gamepads(gamepads);
  Rml::RemoveContext(context->GetName());
  Rml::Shutdown();
  render_interface.Shutdown();
  SDL_ReleaseWindowFromGPUDevice(gpu, window);
  SDL_DestroyGPUDevice(gpu);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
