#pragma once

#include "model.hpp"

#include <gview/imgui_editor.hpp>
#include <gview/sdl3_renderer.hpp>

#include <SDL3/SDL.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct TrialOptions {
    int width = 1280;
    int height = 720;
    int logical_width = 0;
    int logical_height = 0;
    int screen = 0;
    int frames = 0;
    bool hidden = false;
    bool benchmark = false;
    bool self_test = false;
    bool editor = false;
    std::string scenario = "stable";
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
    void resize_host_window(int width, int height);
    void apply_preview(const gview::PreviewConfig& preview);
    void prepare_game_canvas();
    void prepare_tool_layer();
    void process(const SDL_Event& event);
    void update();
    void render();
    void draw_authoring();
    void select_screen(int screen);
    void navigate(gview::NavAction action);
    void enter_text(std::string text);
    void benchmark_step(std::string_view scenario, int frame);
    std::string focus_id() const;
    gview::Value value(std::string_view key) const;
    bool focus_open() const;
    bool authoring_enabled() const;
    void set_authoring_enabled(bool enabled);

    double update_ms() const;
    double render_ms() const;
    double compile_ms() const;
    double activation_ms() const;
    const gview::RuntimeStats& stats() const;
    std::size_t owned_bytes() const;

  private:
    void rebuild();
    void action(std::string_view action, gview::NodeIndex source);
    void load_assets();
    void compile_view(gview::View view, bool reopen_authoring);
    void open_gamepad(SDL_JoystickID id);
    void close_gamepad(SDL_JoystickID id);

    SDL_Renderer* renderer_ = nullptr;
    std::unique_ptr<gview::Sdl3Renderer> painter_;
    std::unordered_map<std::string, SDL_Texture*> textures_;
    std::vector<SDL_Gamepad*> gamepads_;
    TrialModel model_;
    gview::Runtime runtime_;
    gview::AuthoringSession authoring_;
    gview::AuthoringUiState authoring_ui_;
    gview::InputFrame input_;
    int width_ = 1280;
    int height_ = 720;
    int axis_x_ = 0;
    int axis_y_ = 0;
    bool ready_ = false;
    bool running_ = true;
    bool authoring_enabled_ = false;
    bool authored_rebuild_ = false;
    int active_screen_ = 0;
    int active_scenario_ = 0;
    double update_ms_ = 0.0;
    double render_ms_ = 0.0;
    double compile_ms_ = 0.0;
    double activation_ms_ = 0.0;
    float layout_jitter_ = 0.0f;
};

TrialOptions parse_options(int argc, char** argv);
void position_window_on_left_display(SDL_Window* window, int width, int height);
bool run_self_test(TrialApp& app);
