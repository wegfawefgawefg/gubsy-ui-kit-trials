#include "app.hpp"

#include "view_builder.hpp"

#include <SDL3_image/SDL_image.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>

namespace {

using Clock = std::chrono::steady_clock;

double milliseconds(Clock::time_point begin, Clock::time_point end) {
    return std::chrono::duration<double, std::milli>(end - begin).count();
}

std::string asset_path(const char* relative) {
    return std::string(GVIEW_TRIAL_ASSET_DIR) + "/" + relative;
}

// Draws a deliberately native scene behind the renderer-neutral UI surface.
void draw_world_preview(SDL_Renderer* renderer, const gview::PaintCommand& command) {
    const SDL_FRect area{command.rect.x, command.rect.y, command.rect.w, command.rect.h};
    for (int band = 0; band < 48; ++band) {
        const float t = static_cast<float>(band) / 47.0f;
        SDL_SetRenderDrawColor(renderer, static_cast<Uint8>(8.0f + 11.0f * t),
                               static_cast<Uint8>(24.0f + 34.0f * t),
                               static_cast<Uint8>(36.0f + 26.0f * t), 255);
        const SDL_FRect strip{area.x, area.y + area.h * t, area.w, area.h / 47.0f + 1.0f};
        SDL_RenderFillRect(renderer, &strip);
    }
    SDL_SetRenderDrawColor(renderer, 65, 115, 100, 110);
    for (int ridge = 0; ridge < 9; ++ridge) {
        const float y = area.y + area.h * (0.47f + 0.045f * static_cast<float>(ridge));
        SDL_RenderLine(renderer, area.x, y, area.x + area.w * 0.25f, y - 90.0f);
        SDL_RenderLine(renderer, area.x + area.w * 0.25f, y - 90.0f, area.x + area.w * 0.52f,
                       y + 12.0f);
        SDL_RenderLine(renderer, area.x + area.w * 0.52f, y + 12.0f, area.x + area.w, y - 62.0f);
    }
}

} // namespace

// Owns trial model, native runtime, renderer resources, and route transitions.
TrialApp::TrialApp(SDL_Renderer* renderer, int width, int height)
    : renderer_(renderer), width_(width), height_(height) {
    authoring_ui_.preview.width = width;
    authoring_ui_.preview.height = height;
    authoring_ui_.preview.output_width = width;
    authoring_ui_.preview.output_height = height;
    painter_ = std::make_unique<gview::Sdl3Renderer>(renderer_, asset_path("DejaVuSans.ttf"));
    painter_->register_surface("world-preview", draw_world_preview);
    int gamepad_count = 0;
    SDL_JoystickID* gamepads = SDL_GetGamepads(&gamepad_count);
    for (int index = 0; index < gamepad_count; ++index)
        open_gamepad(gamepads[index]);
    SDL_free(gamepads);
    load_assets();
    rebuild();
    ready_ = painter_->ready();
}

TrialApp::~TrialApp() {
    for (SDL_Gamepad* gamepad : gamepads_)
        SDL_CloseGamepad(gamepad);
    for (auto& [id, texture] : textures_) {
        painter_->unregister_texture(id);
        SDL_DestroyTexture(texture);
    }
}

// Owns every connected local controller so SDL emits native gamepad events.
void TrialApp::open_gamepad(SDL_JoystickID id) {
    const auto exists = std::find_if(gamepads_.begin(), gamepads_.end(), [&](SDL_Gamepad* item) {
        return SDL_GetGamepadID(item) == id;
    });
    if (exists != gamepads_.end()) return;
    if (SDL_Gamepad* gamepad = SDL_OpenGamepad(id)) gamepads_.push_back(gamepad);
}

void TrialApp::close_gamepad(SDL_JoystickID id) {
    const auto found = std::find_if(gamepads_.begin(), gamepads_.end(), [&](SDL_Gamepad* item) {
        return SDL_GetGamepadID(item) == id;
    });
    if (found == gamepads_.end()) return;
    SDL_CloseGamepad(*found);
    gamepads_.erase(found);
    axis_x_ = 0;
    axis_y_ = 0;
}

bool TrialApp::ready() const { return ready_; }
bool TrialApp::running() const { return running_; }
void TrialApp::stop() { running_ = false; }

void TrialApp::resize(int width, int height) {
    if (width == width_ && height == height_) return;
    width_ = width;
    height_ = height;
    SDL_SetRenderLogicalPresentation(renderer_, width_, height_,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
    model_.rebuild = true;
}

void TrialApp::resize_host_window(int width, int height) {
    const int host_width = std::max(160, width);
    const int host_height = std::max(144, height);
    if (SDL_Window* window = SDL_GetRenderWindow(renderer_))
        SDL_SetWindowSize(window, host_width, host_height);
}

void TrialApp::apply_preview(const gview::PreviewConfig& preview) {
    resize(preview.width, preview.height);
    SDL_RendererLogicalPresentation presentation = SDL_LOGICAL_PRESENTATION_LETTERBOX;
    if (preview.presentation == gview::PreviewPresentation::Stretch)
        presentation = SDL_LOGICAL_PRESENTATION_STRETCH;
    else if (preview.presentation == gview::PreviewPresentation::Overscan)
        presentation = SDL_LOGICAL_PRESENTATION_OVERSCAN;
    else if (preview.presentation == gview::PreviewPresentation::IntegerScale)
        presentation = SDL_LOGICAL_PRESENTATION_INTEGER_SCALE;
    SDL_SetRenderLogicalPresentation(renderer_, preview.width, preview.height, presentation);
    painter_->set_device_pixel_ratio(preview.device_pixel_ratio);
    painter_->set_nearest_sampling(preview.sampling == gview::PreviewSampling::Nearest);
}

void TrialApp::load_assets() {
    const auto load = [&](std::string id, const std::string& path) {
        SDL_Texture* texture = IMG_LoadTexture(renderer_, path.c_str());
        if (!texture)
            std::fprintf(stderr, "image load failed: %s: %s\n", path.c_str(), SDL_GetError());
        else {
            painter_->register_texture(id, texture);
            textures_.emplace(std::move(id), texture);
        }
    };
    load("splonks-title", asset_path("splonks-title.png"));
    load("ui-panel-grid", std::string(GVIEW_TRIAL_SOURCE_DIR) + "/assets/panel-grid.svg");
    load("ui-control-frame", std::string(GVIEW_TRIAL_SOURCE_DIR) + "/assets/control-frame.svg");
    load("ui-slider-track", std::string(GVIEW_TRIAL_SOURCE_DIR) + "/assets/slider-track.svg");
    load("ui-slider-fill", std::string(GVIEW_TRIAL_SOURCE_DIR) + "/assets/slider-fill.svg");
    load("ui-slider-thumb", std::string(GVIEW_TRIAL_SOURCE_DIR) + "/assets/slider-thumb.svg");
    for (int index = 0; index < 20; ++index) {
        const int sheet = index / 5 + 1;
        const int cell = index % 5;
        load("mod-" + std::to_string(index), asset_path(("mods/sheet-" + std::to_string(sheet) +
                                                         "-" + std::to_string(cell) + ".png")
                                                            .c_str()));
    }
}

void TrialApp::rebuild() {
    gview::View source = model_.game_ui ? build_game_ui_view(model_, width_, height_)
                                        : build_shell_view(model_, width_, height_);
    source.layout.dpi_scale = authoring_ui_.preview.dpi_scale;
    source.layout.form_factor = authoring_ui_.preview.form_factor;
    compile_view(std::move(source), true);
    model_.rebuild = false;
}

void TrialApp::compile_view(gview::View source, bool reopen_authoring) {
    std::string retained_focus;
    if (runtime_.focus() != gview::invalid_node && runtime_.focus() < runtime_.view().nodes.size())
        retained_focus = runtime_.view().nodes[runtime_.focus()].source.layout_id;
    if (reopen_authoring) {
        glayout::graph_canvas_clear(authoring_ui_.canvas);
        authoring_ui_.transaction.reset();
        authoring_ui_.edge_source.clear();
        authoring_ui_.edge_target.clear();
        authoring_.open(source, std::string(GVIEW_TRIAL_SOURCE_DIR) + "/authoring/live-view.sexp");
    }
    const auto compile_begin = Clock::now();
    gview::CompileResult compiled = gview::compile_view(std::move(source));
    compile_ms_ = milliseconds(compile_begin, Clock::now());
    if (!compiled.ok) {
        for (const glayout::Diagnostic& item : compiled.diagnostics)
            std::fprintf(stderr, "view compile: %s\n", item.message.c_str());
        ready_ = false;
        return;
    }
    const auto activation_begin = Clock::now();
    runtime_.reset(std::move(compiled.view));
    activation_ms_ = milliseconds(activation_begin, Clock::now());
    if (!model_.pending_focus.empty()) {
        runtime_.set_focus(model_.pending_focus);
        model_.pending_focus.clear();
    } else if (!retained_focus.empty() && !runtime_.set_focus(retained_focus))
        runtime_.set_focus("nav-" + destination_name(model_.destination));
}

// Applies generic trial actions without placing game meaning inside GView.
void TrialApp::action(std::string_view action_name, gview::NodeIndex) {
    const std::string action(action_name);
    const auto suffix = [&](std::string_view prefix) { return action.substr(prefix.size()); };
    if (action == "quit") running_ = false;
    else if (action == "modal:cancel") {
        model_.modal.clear();
        model_.rebuild = true;
    } else if (action == "modal:confirm") {
        model_.toast = "Destructive change applied";
        model_.modal.clear();
        ++model_.revision;
        model_.rebuild = true;
    } else if (action == "modal:delete") {
        model_.modal = "Delete campaign?";
        model_.pending_focus = "modal-cancel";
        model_.rebuild = true;
    } else if (action == "modal:uninstall") {
        model_.modal = "Uninstall dependent packages?";
        model_.pending_focus = "modal-cancel";
        model_.rebuild = true;
    } else if (action.rfind("destination:", 0) == 0) {
        const std::string name = suffix("destination:");
        if (name == "Play") model_.destination = Destination::Play;
        else if (name == "Players") model_.destination = Destination::Players;
        else if (name == "Settings") model_.destination = Destination::Settings;
        else if (name == "Controls") model_.destination = Destination::Controls;
        else if (name == "Progress") model_.destination = Destination::Progress;
        else if (name == "Mods") model_.destination = Destination::Mods;
        model_.rebuild = true;
    } else if (action.rfind("players-tab:", 0) == 0) {
        model_.players_tab = suffix("players-tab:");
        model_.rebuild = true;
    } else if (action.rfind("settings-tab:", 0) == 0) {
        model_.settings_tab = suffix("settings-tab:");
        model_.rebuild = true;
    } else if (action.rfind("controls-tab:", 0) == 0) {
        model_.controls_tab = suffix("controls-tab:");
        model_.rebuild = true;
    } else if (action.rfind("mods-tab:", 0) == 0) {
        model_.mods_tab = suffix("mods-tab:");
        model_.rebuild = true;
    } else if (action == "play:lobby") {
        model_.destination = Destination::Play;
        model_.play_page = PlayPage::Lobby;
        model_.rebuild = true;
    } else if (action == "play:quest") {
        model_.play_page = PlayPage::Quest;
        model_.rebuild = true;
    } else if (action == "play:rules") {
        model_.play_page = PlayPage::Rules;
        model_.rebuild = true;
    } else if (action == "play:mods") {
        model_.play_page = PlayPage::SessionMods;
        model_.rebuild = true;
    } else if (action == "mods:browse") {
        model_.destination = Destination::Mods;
        model_.mods_tab = "Browse catalog";
        model_.rebuild = true;
    } else if (action == "controls:devices") {
        model_.destination = Destination::Controls;
        model_.controls_tab = "Devices";
        model_.rebuild = true;
    } else if (action.rfind("select:", 0) == 0) {
        model_.selected = suffix("select:");
        ++model_.revision;
    } else if (action == "toast:clear") {
        model_.toast.clear();
        model_.rebuild = true;
    } else if (action.rfind("toast:", 0) == 0) {
        model_.toast = suffix("toast:");
        ++model_.revision;
        model_.rebuild = true;
    } else if (action == "back" && !model_.modal.empty()) {
        model_.modal.clear();
        model_.rebuild = true;
    } else if (action == "back" && model_.destination == Destination::Play &&
               model_.play_page != PlayPage::Lobby) {
        model_.play_page = PlayPage::Lobby;
        model_.rebuild = true;
    } else if (!action.empty()) {
        model_.toast = "Action: " + action;
        ++model_.revision;
        model_.rebuild = true;
    }
}

void TrialApp::update() {
    if (model_.rebuild) rebuild();
    else if (authored_rebuild_) {
        compile_view(authoring_.view(), false);
        authored_rebuild_ = false;
    }
    const auto begin = Clock::now();
    gview::Host host;
    host.read = [&](std::string_view key) { return model_.read(key); };
    host.write = [&](std::string_view key, const gview::Value& value) { model_.write(key, value); };
    host.action = [&](std::string_view name, gview::NodeIndex source) { action(name, source); };
    host.revision = model_.revision;
    glayout::ResolveInput resolution{glayout::Rect{layout_jitter_, 0.0f,
                                                   static_cast<float>(width_) - layout_jitter_,
                                                   static_cast<float>(height_)},
                                     authoring_ui_.preview.safe_area, nullptr, nullptr};
    runtime_.frame(resolution, input_, host);
    input_ = {};
    update_ms_ = milliseconds(begin, Clock::now());
}

void TrialApp::render() {
    const auto begin = Clock::now();
    SDL_SetRenderDrawColor(renderer_, 4, 15, 18, 255);
    SDL_RenderClear(renderer_);
    painter_->render(runtime_.paint());
    render_ms_ = milliseconds(begin, Clock::now());
}

void TrialApp::draw_authoring() {
    if (!authoring_enabled_) return;
    gview::AuthoringHooks hooks;
    hooks.states = {"Play / lobby",
                    "Play / quest",
                    "Play / rules",
                    "Play / session mods",
                    "Players / local",
                    "Players / profiles",
                    "Players / devices",
                    "Settings / display",
                    "Settings / audio",
                    "Settings / accessibility",
                    "Settings / gameplay",
                    "Controls / bindings",
                    "Controls / devices",
                    "Controls / tuning",
                    "Progress",
                    "Mods / installed",
                    "Mods / catalog",
                    "In-game inventory"};
    hooks.active_state = active_screen_;
    hooks.scenarios = {"Populated", "Loading", "Empty", "Provider error"};
    hooks.active_scenario = active_scenario_;
    hooks.default_save_path = std::string(GVIEW_TRIAL_SOURCE_DIR) + "/authoring/live-view.sexp";
    hooks.select_state = [&](int screen) { select_screen(screen); };
    hooks.select_scenario = [&](int scenario) {
        active_scenario_ = std::clamp(scenario, 0, 3);
        model_.provider_state = hooks.scenarios[static_cast<std::size_t>(active_scenario_)];
        model_.rebuild = true;
    };
    hooks.apply_preview = [&](const gview::PreviewConfig& preview) { apply_preview(preview); };
    hooks.resize_host_window = [&](int width, int height) { resize_host_window(width, height); };
    hooks.rebuild = [&] { authored_rebuild_ = true; };
    hooks.metrics = [&] {
        char text[256]{};
        std::snprintf(text, sizeof(text),
                      "update %.3f ms  paint/backend %.3f ms\n%zu nodes  %zu "
                      "commands  %.1f KiB owned",
                      update_ms_, render_ms_, runtime_.view().nodes.size(), runtime_.paint().size(),
                      static_cast<double>(runtime_.owned_bytes()) / 1024.0);
        return std::string(text);
    };
    gview::draw_authoring_tools(authoring_, authoring_ui_, hooks, runtime_.view(),
                                runtime_.geometry());
}

void TrialApp::select_screen(int screen) {
    active_screen_ = std::clamp(screen, 0, 17);
    screen = active_screen_;
    model_.game_ui = screen == 17;
    if (model_.game_ui) {
        model_.pending_focus = "inventory-item-8";
        model_.rebuild = true;
        return;
    }
    if (screen < 4) {
        model_.destination = Destination::Play;
        model_.play_page = static_cast<PlayPage>(screen);
    } else if (screen < 7) {
        model_.destination = Destination::Players;
        model_.players_tab =
            std::array<std::string, 3>{"Local players", "Profiles", "Devices"}[screen - 4];
    } else if (screen < 11) {
        model_.destination = Destination::Settings;
        model_.settings_tab =
            std::array<std::string, 4>{"Display", "Audio", "Accessibility", "Gameplay"}[screen - 7];
    } else if (screen < 14) {
        model_.destination = Destination::Controls;
        model_.controls_tab =
            std::array<std::string, 3>{"Bindings", "Devices", "Input tuning"}[screen - 11];
    } else if (screen == 14) model_.destination = Destination::Progress;
    else {
        model_.destination = Destination::Mods;
        model_.mods_tab = screen == 16 ? "Browse catalog" : "Installed";
    }
    model_.pending_focus = "nav-" + destination_name(model_.destination);
    model_.rebuild = true;
}

void TrialApp::navigate(gview::NavAction action) { input_.navigation.push_back(action); }
void TrialApp::enter_text(std::string text) { input_.text += std::move(text); }

void TrialApp::benchmark_step(std::string_view scenario, int frame) {
    if (scenario == "value")
        model_.write("brightness", frame % 2 == 0 ? gview::Value{0.63} : gview::Value{0.64});
    else if (scenario == "layout") layout_jitter_ = frame % 2 == 0 ? 0.0f : 1.0f;
    else if (scenario == "scroll") {
        const auto found = runtime_.view().indices.find("catalog-list");
        if (found != runtime_.view().indices.end()) {
            const gview::CompiledNode& node = runtime_.view().nodes[found->second];
            if (node.layout_index < runtime_.geometry().size()) {
                const glayout::Rect area = runtime_.geometry()[node.layout_index].border;
                input_.pointer.x = area.x + area.w * 0.5f;
                input_.pointer.y = area.y + area.h * 0.5f;
                input_.pointer.scroll_y = (frame / 24) % 2 == 0 ? -1.0f : 1.0f;
            }
        }
    }
}

std::string TrialApp::focus_id() const {
    const gview::NodeIndex focus = runtime_.focus();
    if (focus == gview::invalid_node || focus >= runtime_.view().nodes.size()) return {};
    return runtime_.view().nodes[focus].source.layout_id;
}

gview::Value TrialApp::value(std::string_view key) const { return model_.read(key); }

bool TrialApp::focus_open() const {
    const gview::NodeIndex focus = runtime_.focus();
    return focus != gview::invalid_node && focus < runtime_.state().size() &&
           runtime_.state()[focus].open;
}

bool TrialApp::authoring_enabled() const { return authoring_enabled_; }
void TrialApp::set_authoring_enabled(bool enabled) { authoring_enabled_ = enabled; }

double TrialApp::update_ms() const { return update_ms_; }
double TrialApp::render_ms() const { return render_ms_; }
double TrialApp::compile_ms() const { return compile_ms_; }
double TrialApp::activation_ms() const { return activation_ms_; }
const gview::RuntimeStats& TrialApp::stats() const { return runtime_.stats(); }
std::size_t TrialApp::owned_bytes() const { return runtime_.owned_bytes(); }
