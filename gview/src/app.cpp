#include "app.hpp"

#include "view_builder.hpp"

#include <SDL3_image/SDL_image.h>

#include <chrono>
#include <array>
#include <cstdio>

namespace {

using Clock = std::chrono::steady_clock;

double milliseconds(Clock::time_point begin, Clock::time_point end) {
    return std::chrono::duration<double, std::milli>(end - begin).count();
}

std::string asset_path(const char* relative) {
    return std::string(GVIEW_TRIAL_ASSET_DIR) + "/" + relative;
}

} // namespace

// Owns trial model, native runtime, renderer resources, and route transitions.
TrialApp::TrialApp(SDL_Renderer* renderer, int width, int height)
    : renderer_(renderer), width_(width), height_(height) {
    painter_ = std::make_unique<gview::Sdl3Renderer>(renderer_, asset_path("DejaVuSans.ttf"));
    load_assets();
    rebuild();
    ready_ = painter_->ready();
}

TrialApp::~TrialApp() {
    for (auto& [id, texture] : textures_) {
        painter_->unregister_texture(id);
        SDL_DestroyTexture(texture);
    }
}

bool TrialApp::ready() const { return ready_; }
bool TrialApp::running() const { return running_; }
void TrialApp::stop() { running_ = false; }

void TrialApp::resize(int width, int height) {
    if (width == width_ && height == height_) return;
    width_ = width;
    height_ = height;
    SDL_SetRenderLogicalPresentation(renderer_, width_, height_, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    model_.rebuild = true;
}

void TrialApp::load_assets() {
    const auto load = [&](std::string id, const std::string& path) {
        SDL_Texture* texture = IMG_LoadTexture(renderer_, path.c_str());
        if (!texture) std::fprintf(stderr, "image load failed: %s: %s\n", path.c_str(), SDL_GetError());
        else {
            painter_->register_texture(id, texture);
            textures_.emplace(std::move(id), texture);
        }
    };
    load("splonks-title", asset_path("splonks-title.png"));
    for (int index = 0; index < 20; ++index) {
        const int sheet = index / 5 + 1;
        const int cell = index % 5;
        load("mod-" + std::to_string(index),
             asset_path(("mods/sheet-" + std::to_string(sheet) + "-" + std::to_string(cell) + ".png").c_str()));
    }
}

void TrialApp::rebuild() {
    std::string retained_focus;
    if (runtime_.focus() != gview::invalid_node &&
        runtime_.focus() < runtime_.view().nodes.size())
        retained_focus = runtime_.view().nodes[runtime_.focus()].source.layout_id;
    gview::CompileResult compiled = gview::compile_view(build_shell_view(model_, width_, height_));
    if (!compiled.ok) {
        for (const glayout::Diagnostic& item : compiled.diagnostics)
            std::fprintf(stderr, "view compile: %s\n", item.message.c_str());
        ready_ = false;
        return;
    }
    runtime_.reset(std::move(compiled.view));
    if (!model_.pending_focus.empty()) {
        runtime_.set_focus(model_.pending_focus);
        model_.pending_focus.clear();
    } else if (!retained_focus.empty() && !runtime_.set_focus(retained_focus))
        runtime_.set_focus("nav-" + destination_name(model_.destination));
    model_.rebuild = false;
}

// Applies generic trial actions without placing game meaning inside GView.
void TrialApp::action(std::string_view action_name, gview::NodeIndex) {
    const std::string action(action_name);
    const auto suffix = [&](std::string_view prefix) { return action.substr(prefix.size()); };
    if (action == "quit") running_ = false;
    else if (action == "modal:cancel") {
        model_.modal.clear(); model_.rebuild = true;
    } else if (action == "modal:confirm") {
        model_.toast = "Destructive change applied";
        model_.modal.clear(); ++model_.revision; model_.rebuild = true;
    } else if (action == "modal:delete") {
        model_.modal = "Delete campaign?";
        model_.pending_focus = "modal-cancel";
        model_.rebuild = true;
    } else if (action == "modal:uninstall") {
        model_.modal = "Uninstall dependent packages?";
        model_.pending_focus = "modal-cancel";
        model_.rebuild = true;
    }
    else if (action.rfind("destination:", 0) == 0) {
        const std::string name = suffix("destination:");
        if (name == "Play") model_.destination = Destination::Play;
        else if (name == "Players") model_.destination = Destination::Players;
        else if (name == "Settings") model_.destination = Destination::Settings;
        else if (name == "Controls") model_.destination = Destination::Controls;
        else if (name == "Progress") model_.destination = Destination::Progress;
        else if (name == "Mods") model_.destination = Destination::Mods;
        model_.rebuild = true;
    } else if (action.rfind("players-tab:", 0) == 0) {
        model_.players_tab = suffix("players-tab:"); model_.rebuild = true;
    } else if (action.rfind("settings-tab:", 0) == 0) {
        model_.settings_tab = suffix("settings-tab:"); model_.rebuild = true;
    } else if (action.rfind("controls-tab:", 0) == 0) {
        model_.controls_tab = suffix("controls-tab:"); model_.rebuild = true;
    } else if (action.rfind("mods-tab:", 0) == 0) {
        model_.mods_tab = suffix("mods-tab:"); model_.rebuild = true;
    } else if (action == "play:lobby") {
        model_.destination = Destination::Play; model_.play_page = PlayPage::Lobby; model_.rebuild = true;
    } else if (action == "play:quest") {
        model_.play_page = PlayPage::Quest; model_.rebuild = true;
    } else if (action == "play:rules") {
        model_.play_page = PlayPage::Rules; model_.rebuild = true;
    } else if (action == "play:mods") {
        model_.play_page = PlayPage::SessionMods; model_.rebuild = true;
    } else if (action == "mods:browse") {
        model_.destination = Destination::Mods; model_.mods_tab = "Browse catalog"; model_.rebuild = true;
    } else if (action == "controls:devices") {
        model_.destination = Destination::Controls; model_.controls_tab = "Devices"; model_.rebuild = true;
    } else if (action.rfind("select:", 0) == 0) {
        model_.selected = suffix("select:"); ++model_.revision;
    } else if (action.rfind("toast:", 0) == 0) {
        model_.toast = suffix("toast:"); ++model_.revision;
    } else if (action == "back" && !model_.modal.empty()) {
        model_.modal.clear(); model_.rebuild = true;
    } else if (action == "back" && model_.destination == Destination::Play &&
               model_.play_page != PlayPage::Lobby) {
        model_.play_page = PlayPage::Lobby; model_.rebuild = true;
    }
}

void TrialApp::update() {
    if (model_.rebuild) rebuild();
    const auto begin = Clock::now();
    gview::Host host;
    host.read = [&](std::string_view key) { return model_.read(key); };
    host.write = [&](std::string_view key, const gview::Value& value) { model_.write(key, value); };
    host.action = [&](std::string_view name, gview::NodeIndex source) { action(name, source); };
    host.revision = model_.revision;
    glayout::ResolveInput resolution{glayout::Rect{0.0f, 0.0f, static_cast<float>(width_),
                                                   static_cast<float>(height_)},
                                     {}, nullptr, nullptr};
    runtime_.frame(resolution, input_, host);
    input_ = {};
    update_ms_ = milliseconds(begin, Clock::now());
}

void TrialApp::render() {
    const auto begin = Clock::now();
    SDL_SetRenderDrawColor(renderer_, 4, 15, 18, 255);
    SDL_RenderClear(renderer_);
    painter_->render(runtime_.paint());
    SDL_RenderPresent(renderer_);
    render_ms_ = milliseconds(begin, Clock::now());
}

void TrialApp::select_screen(int screen) {
    if (screen < 4) {
        model_.destination = Destination::Play;
        model_.play_page = static_cast<PlayPage>(screen);
    } else if (screen < 7) {
        model_.destination = Destination::Players;
        model_.players_tab = std::array<std::string, 3>{"Local players", "Profiles", "Devices"}[screen - 4];
    } else if (screen < 11) {
        model_.destination = Destination::Settings;
        model_.settings_tab = std::array<std::string, 4>{"Display", "Audio", "Accessibility", "Gameplay"}[screen - 7];
    } else if (screen < 14) {
        model_.destination = Destination::Controls;
        model_.controls_tab = std::array<std::string, 3>{"Bindings", "Devices", "Input tuning"}[screen - 11];
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

double TrialApp::update_ms() const { return update_ms_; }
double TrialApp::render_ms() const { return render_ms_; }
const gview::RuntimeStats& TrialApp::stats() const { return runtime_.stats(); }
