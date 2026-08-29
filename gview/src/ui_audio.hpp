#pragma once

#include <gview/types.hpp>

#include <memory>

// Plays trial feedback without making GView own an audio backend.
class UiAudio {
  public:
    UiAudio();
    ~UiAudio();

    UiAudio(const UiAudio&) = delete;
    UiAudio& operator=(const UiAudio&) = delete;

    void play(gview::FeedbackEvent event);

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
