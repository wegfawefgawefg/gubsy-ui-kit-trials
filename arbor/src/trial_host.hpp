#pragma once

#include "benchmark.hpp"
#include "trial_app.hpp"
#include "trial_options.hpp"

#include <arbor/app/host.hpp>
#include <arbor/sdl3_gpu/gpu_renderer.hpp>
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class TrialHost {
  public:
    explicit TrialHost(TrialOptions options);
    int run();

  private:
    void handle_action(std::string_view action, const arbor::widgets::WidgetEvent& event);
    bool switch_screen(std::string_view destination);
    void refresh_screens();

    TrialOptions options_;
    TrialModel model_;
    std::array<std::unique_ptr<TrialScreen>, 6> screens_;
    std::array<std::unique_ptr<arbor::app::ApplicationHost>, 6> hosts_;
    std::size_t active_screen_{};
    std::optional<std::string> pending_screen_;
    BenchmarkSamples samples_;
};
