#include "RmlUi_Platform_SDL.h"
#include "RmlUi_Renderer_SDL_GPU.h"

#include <RmlUi/Core.h>
#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <string_view>

namespace {

using Clock = std::chrono::steady_clock;

struct Timings {
  double update_ms = 0.0;
  double render_record_ms = 0.0;
  double submit_ms = 0.0;
  double frame_ms = 0.0;
};

struct Resolution {
  const char* label;
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

constexpr std::array<const char*, 8> kTargetScreens{{
    "Play lobby", "Quest picker", "Session settings", "Session mods",
    "Players", "Controls", "Progress", "Mod catalog",
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

void print_help(const char* executable) {
  std::printf(
      "Usage: %s [--resolution WIDTHxHEIGHT] [--hidden] [--frames N]\n"
      "\n"
      "F1 toggles the native experiment panel. Escape exits.\n"
      "--frames is useful for automated smoke runs under a display server.\n",
      executable);
}

bool parse_resolution(std::string_view value, int& width, int& height) {
  const size_t split = value.find('x');
  if (split == std::string_view::npos) return false;
  width = std::atoi(std::string(value.substr(0, split)).c_str());
  height = std::atoi(std::string(value.substr(split + 1)).c_str());
  return width > 0 && height > 0;
}

void apply_document_state(Rml::ElementDocument* document, int screen_index,
                          int width, int height) {
  if (!document) return;
  if (Rml::Element* target = document->GetElementById("target-name"))
    target->SetInnerRML(kTargetScreens[screen_index]);
  if (Rml::Element* viewport = document->GetElementById("viewport-size"))
    viewport->SetInnerRML(std::to_string(width) + " × " + std::to_string(height));
}

}  // namespace

int main(int argc, char** argv) {
  int window_width = 1280;
  int window_height = 720;
  int frame_limit = 0;
  bool hidden = false;

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

  SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
  if (hidden) flags |= SDL_WINDOW_HIDDEN;
  SDL_Window* window = SDL_CreateWindow("Gubsy RmlUi Design Demo", window_width,
                                        window_height, flags);
  if (!window) {
    std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }
  SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

  SDL_GPUDevice* gpu = SDL_CreateGPUDevice(
      SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL |
          SDL_GPU_SHADERFORMAT_MSL,
      true, nullptr);
  if (!gpu || !SDL_ClaimWindowForGPUDevice(gpu, window)) {
    std::fprintf(stderr, "SDL GPU setup failed: %s\n", SDL_GetError());
    if (gpu) SDL_DestroyGPUDevice(gpu);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }
  SDL_SetGPUSwapchainParameters(gpu, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
                                SDL_GPU_PRESENTMODE_VSYNC);

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

  int drawable_width = 0;
  int drawable_height = 0;
  SDL_GetWindowSizeInPixels(window, &drawable_width, &drawable_height);
  Rml::Context* context = Rml::CreateContext(
      "gubsy-shell", Rml::Vector2i(drawable_width, drawable_height));
  if (!context) {
    std::fprintf(stderr, "Could not create RmlUi context.\n");
    return 1;
  }
  const std::string document_path =
      std::string(GUBSY_UI_SOURCE_ASSET_DIR) + "/ui/shell.rml";
  Rml::ElementDocument* document = context->LoadDocument(document_path);
  if (!document) {
    std::fprintf(stderr, "Could not load document: %s\n", document_path.c_str());
    return 1;
  }
  document->Show();

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
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
  bool show_tools = true;
  int selected_resolution = 1;
  int selected_screen = 0;
  float dp_ratio = 1.0f;
  int completed_frames = 0;
  Timings timings;
  apply_document_state(document, selected_screen, drawable_width, drawable_height);

  while (running) {
    const auto frame_start = Clock::now();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);
      if (event.type == SDL_EVENT_QUIT) running = false;
      if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)
        running = false;
      if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_F1)
        show_tools = !show_tools;
      RmlSDL::InputEventHandler(context, window, event);
    }

    int next_width = 0;
    int next_height = 0;
    SDL_GetWindowSizeInPixels(window, &next_width, &next_height);
    if (next_width != drawable_width || next_height != drawable_height) {
      drawable_width = next_width;
      drawable_height = next_height;
      context->SetDimensions({drawable_width, drawable_height});
      apply_document_state(document, selected_screen, drawable_width,
                           drawable_height);
    }

    const auto update_start = Clock::now();
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
      if (ImGui::Combo("Target screen", &selected_screen,
                       kTargetScreens.data(), kTargetScreens.size()))
        apply_document_state(document, selected_screen, drawable_width,
                             drawable_height);
      if (ImGui::Combo("Resolution", &selected_resolution,
                       [](void*, int index) { return kResolutions[index].label; },
                       nullptr, kResolutions.size())) {
        const Resolution& resolution = kResolutions[selected_resolution];
        SDL_SetWindowSize(window, resolution.width, resolution.height);
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED);
      }
      if (ImGui::SliderFloat("RmlUi dp ratio", &dp_ratio, 0.5f, 3.0f,
                             "%.2f"))
        context->SetDensityIndependentPixelRatio(dp_ratio);
      ImGui::Separator();
      ImGui::Text("Rml update: %.3f ms", timings.update_ms);
      ImGui::Text("Render record: %.3f ms", timings.render_record_ms);
      ImGui::Text("GPU submit: %.3f ms", timings.submit_ms);
      ImGui::Text("CPU frame: %.3f ms", timings.frame_ms);
      ImGui::Text("RSS: %ld KiB", read_rss_kib());
      ImGui::Text("GPU: %s", SDL_GetGPUDeviceDriver(gpu));
      ImGui::End();
    }
    ImGui::Render();

    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(gpu);
    if (!command_buffer) {
      std::fprintf(stderr, "Could not acquire command buffer: %s\n",
                   SDL_GetError());
      break;
    }
    SDL_GPUTexture* swapchain = nullptr;
    uint32_t swap_width = 0;
    uint32_t swap_height = 0;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(
            command_buffer, window, &swapchain, &swap_width, &swap_height)) {
      std::fprintf(stderr, "Could not acquire swapchain: %s\n", SDL_GetError());
      SDL_CancelGPUCommandBuffer(command_buffer);
      break;
    }
    if (swapchain) {
      SDL_GPUColorTargetInfo clear_target{};
      clear_target.texture = swapchain;
      clear_target.clear_color = {0.012f, 0.025f, 0.028f, 1.0f};
      clear_target.load_op = SDL_GPU_LOADOP_CLEAR;
      clear_target.store_op = SDL_GPU_STOREOP_STORE;
      SDL_GPURenderPass* clear_pass =
          SDL_BeginGPURenderPass(command_buffer, &clear_target, 1, nullptr);
      SDL_EndGPURenderPass(clear_pass);

      const auto render_start = Clock::now();
      render_interface.BeginFrame(command_buffer, swapchain, swap_width,
                                  swap_height);
      context->Render();
      render_interface.EndFrame();
      ImDrawData* draw_data = ImGui::GetDrawData();
      ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, command_buffer);
      SDL_GPUColorTargetInfo imgui_target{};
      imgui_target.texture = swapchain;
      imgui_target.load_op = SDL_GPU_LOADOP_LOAD;
      imgui_target.store_op = SDL_GPU_STOREOP_STORE;
      SDL_GPURenderPass* imgui_pass =
          SDL_BeginGPURenderPass(command_buffer, &imgui_target, 1, nullptr);
      ImGui_ImplSDLGPU3_RenderDrawData(draw_data, command_buffer, imgui_pass);
      SDL_EndGPURenderPass(imgui_pass);
      timings.render_record_ms = milliseconds(render_start, Clock::now());
    }

    const auto submit_start = Clock::now();
    SDL_SubmitGPUCommandBuffer(command_buffer);
    timings.submit_ms = milliseconds(submit_start, Clock::now());
    timings.frame_ms = milliseconds(frame_start, Clock::now());

    ++completed_frames;
    if (frame_limit > 0 && completed_frames >= frame_limit) running = false;
  }

  SDL_WaitForGPUIdle(gpu);
  ImGui_ImplSDLGPU3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
  Rml::RemoveContext(context->GetName());
  Rml::Shutdown();
  render_interface.Shutdown();
  SDL_ReleaseWindowFromGPUDevice(gpu, window);
  SDL_DestroyGPUDevice(gpu);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
