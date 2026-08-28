#pragma once

#include "model.hpp"

#include <gview/sdl3_renderer.hpp>

#include <SDL3/SDL.h>

#include <memory>
#include <string>
#include <unordered_map>

struct TrialOptions {
    int width = 1280;
    int height = 720;
    int screen = 0;
    int frames = 0;
    bool hidden = false;
    bool benchmark = false;
    bool self_test = false;
    std::string capture;
};

class TrialApp {
  public:
    TrialApp(SDL_Renderer* renderer, int width, int height);
    ~TrialApp();

    bool ready() const;
    bool running() const;
    void stop();
    void resize(int width, int height);
    void process(const SDL_Event& event);
    void update();
    void render();
    void select_screen(int screen);
    void navigate(gview::NavAction action);
    void enter_text(std::string text);
    std::string focus_id() const;
    gview::Value value(std::string_view key) const;
    bool focus_open() const;

    double update_ms() const;
    double render_ms() const;
    const gview::RuntimeStats& stats() const;

  private:
    void rebuild();
    void action(std::string_view action, gview::NodeIndex source);
    void load_assets();

    SDL_Renderer* renderer_ = nullptr;
    std::unique_ptr<gview::Sdl3Renderer> painter_;
    std::unordered_map<std::string, SDL_Texture*> textures_;
    TrialModel model_;
    gview::Runtime runtime_;
    gview::InputFrame input_;
    int width_ = 1280;
    int height_ = 720;
    int axis_x_ = 0;
    int axis_y_ = 0;
    bool ready_ = false;
    bool running_ = true;
    double update_ms_ = 0.0;
    double render_ms_ = 0.0;
};

TrialOptions parse_options(int argc, char** argv);
void position_window_on_left_display(SDL_Window* window, int width, int height);
bool run_self_test(TrialApp& app);
