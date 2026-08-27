#pragma once

#include <cstddef>
#include <string>

struct TrialOptions {
    int width{1280};
    int height{720};
    std::string screen{"play"};
    std::size_t benchmark_frames{};
    bool hidden{};
};

bool parse_trial_options(int argc, char** argv, TrialOptions& options);
