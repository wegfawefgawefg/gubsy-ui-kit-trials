#include "trial_options.hpp"

#include <charconv>
#include <cstdio>
#include <string_view>

namespace {

bool parse_positive(std::string_view text, int& value) {
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    return result.ec == std::errc{} && result.ptr == text.data() + text.size() && value > 0;
}

bool parse_resolution(std::string_view text, int& width, int& height) {
    // parse WIDTHxHEIGHT
    const std::size_t split = text.find('x');
    return split != std::string_view::npos && parse_positive(text.substr(0, split), width) &&
           parse_positive(text.substr(split + 1), height);
}

} // namespace

bool parse_trial_options(int argc, char** argv, TrialOptions& options) {
    // apply explicit trial options
    for (int index = 1; index < argc; ++index) {
        const std::string_view argument{argv[index]};
        if (argument == "--resolution" && index + 1 < argc) {
            if (!parse_resolution(argv[++index], options.width, options.height)) {
                std::fprintf(stderr, "invalid resolution; expected WIDTHxHEIGHT\n");
                return false;
            }
        } else if (argument == "--screen" && index + 1 < argc) {
            options.screen = argv[++index];
        } else if (argument == "--benchmark" && index + 1 < argc) {
            int frames = 0;
            if (!parse_positive(argv[++index], frames)) {
                std::fprintf(stderr, "invalid benchmark frame count\n");
                return false;
            }
            options.benchmark_frames = static_cast<std::size_t>(frames);
            options.hidden = true;
        } else if (argument == "--hidden") {
            options.hidden = true;
        } else if (argument == "--help" || argument == "-h") {
            std::printf("usage: gubsy-arbor-trial [--resolution WxH] "
                        "[--screen ROUTE] [--benchmark N] [--hidden]\n");
            return false;
        } else {
            std::fprintf(stderr, "unknown argument: %s\n", argv[index]);
            return false;
        }
    }
    return true;
}
