#include "trial_options.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>

namespace {

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
  // parse WIDTHxHEIGHT
  const size_t split = value.find('x');
  if (split == std::string_view::npos)
    return false;
  width = std::atoi(std::string(value.substr(0, split)).c_str());
  height = std::atoi(std::string(value.substr(split + 1)).c_str());
  return width > 0 && height > 0;
}

} // namespace

ParseResult parse_trial_options(int argc, char **argv, TrialOptions &options) {
  // apply explicit trial options
  for (int i = 1; i < argc; ++i) {
    const std::string_view argument(argv[i]);
    if (argument == "--help" || argument == "-h") {
      print_help(argv[0]);
      return ParseResult::ExitSuccess;
    }
    if (argument == "--hidden") {
      options.hidden = true;
      continue;
    }
    if (argument == "--no-tools") {
      options.tools_visible = false;
      continue;
    }
    if (argument == "--benchmark" && i + 1 < argc) {
      options.benchmark = true;
      options.hidden = true;
      options.tools_visible = false;
      options.frame_limit = std::atoi(argv[++i]);
      if (options.frame_limit <= 120) {
        std::fprintf(stderr, "Benchmark needs more than 120 frames for warmup.\n");
        return ParseResult::ExitFailure;
      }
      continue;
    }
    if (argument == "--capture" && i + 1 < argc) {
      options.capture_path = argv[++i];
      options.hidden = true;
      options.tools_visible = false;
      if (options.frame_limit == 0)
        options.frame_limit = 5;
      continue;
    }
    if (argument == "--self-test") {
      options.self_test = true;
      options.hidden = true;
      options.tools_visible = false;
      if (options.frame_limit == 0)
        options.frame_limit = 5;
      continue;
    }
    if (argument == "--screen" && i + 1 < argc) {
      options.initial_screen = std::atoi(argv[++i]);
      if (options.initial_screen < 0 ||
          options.initial_screen >= static_cast<int>(kTargetScreens.size())) {
        std::fprintf(stderr, "Invalid screen index. Expected 0 through 16.\n");
        return ParseResult::ExitFailure;
      }
      continue;
    }
    if (argument == "--provider" && i + 1 < argc) {
      options.initial_provider_state = std::atoi(argv[++i]);
      if (options.initial_provider_state < 0 ||
          options.initial_provider_state >=
              static_cast<int>(kProviderStates.size())) {
        std::fprintf(stderr,
                     "Invalid provider state. Expected 0 through 3.\n");
        return ParseResult::ExitFailure;
      }
      continue;
    }
    if (argument == "--resolution" && i + 1 < argc) {
      if (!parse_resolution(argv[++i], options.window_width,
                            options.window_height)) {
        std::fprintf(stderr, "Invalid resolution. Expected WIDTHxHEIGHT.\n");
        return ParseResult::ExitFailure;
      }
      continue;
    }
    if (argument == "--frames" && i + 1 < argc) {
      options.frame_limit = std::atoi(argv[++i]);
      continue;
    }
    std::fprintf(stderr, "Unknown argument: %s\n", argv[i]);
    return ParseResult::ExitFailure;
  }
  return ParseResult::Run;
}
