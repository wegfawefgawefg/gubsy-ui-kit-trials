#pragma once

#include "trial_model.hpp"

#include <arbor/app/app_client.hpp>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>

class TrialScreen final : public arbor::app::AppClient {
  public:
    using ActionHandler = std::function<void(std::string_view, const arbor::widgets::WidgetEvent&)>;

    TrialScreen(std::filesystem::path source, std::filesystem::path components,
                std::filesystem::path font, TrialModel& model, ActionHandler handler);

    arbor::Result<void> initialize() override;
    void refresh() noexcept {
        mark_dirty();
    }
    const arbor::core::FrameProfile& profile() const noexcept;

  private:
    std::filesystem::path components_;
    TrialModel& model_;
};
