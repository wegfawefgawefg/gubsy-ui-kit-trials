#include "trial_host.hpp"

#include "controller_input.hpp"

#include <SDL3/SDL.h>
#include <algorithm>
#include <arbor/sdl3_gpu/input.hpp>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <string_view>

namespace {

constexpr std::array<std::string_view, 6> kRoutes{"play",     "players",  "settings",
                                                  "controls", "progress", "mods"};

double milliseconds(arbor::core::Duration duration) {
    return std::chrono::duration<double, std::milli>(duration).count();
}

double milliseconds(std::chrono::steady_clock::time_point begin,
                    std::chrono::steady_clock::time_point end) {
    return std::chrono::duration<double, std::milli>(end - begin).count();
}

void print_diagnostics(std::span<const arbor::Diagnostic> diagnostics) {
    for (const auto& diagnostic : diagnostics)
        std::fprintf(stderr, "arbor trial: %s\n", diagnostic.message.c_str());
}

} // namespace

TrialHost::TrialHost(TrialOptions options) : options_(std::move(options)) {
}

void TrialHost::handle_action(std::string_view action, const arbor::widgets::WidgetEvent& event) {
    // Composite a-model handlers are compiler-generated. Route any known shell
    // destination payload after the active event batch, independent of its
    // generated symbol name.
    if (event.payload.is_string()) {
        const std::string_view payload = event.payload.as_string();
        if (std::find(kRoutes.begin(), kRoutes.end(), payload) != kRoutes.end()) {
            pending_screen_ = std::string{payload};
            return;
        }
    }

    // Open the substantial Play workflows without leaving the Play destination.
    if (action == "checkpoint")
        model_.play_view = "checkpoint";
    else if (action == "rules")
        model_.play_view = "rules";
    else if (action == "sessionMods")
        model_.play_view = "sessionMods";
    else if (action == "playBack")
        model_.play_view = "lobby";
    else if (action == "browseMods") {
        model_.mods_tab = "catalog";
        pending_screen_ = "mods";
    } else if (action == "installSession")
        model_.mod_action_status = "INSTALLED AND ADDED TO SESSION";
    else if (action == "installOnly")
        model_.mod_action_status = "INSTALLED LOCALLY";
    else if (action == "updateMod")
        model_.mod_action_status = "UPDATED";

    // update selected authored records
    else if (action == "profileMoss")
        model_.selected_profile = "Moss";
    else if (action == "profileVega")
        model_.selected_profile = "Vega";
    else if (action == "profileGuest")
        model_.selected_profile = "Guest";
    else if (action == "campaignGlass")
        model_.selected_campaign = "The Glass Caverns";
    else if (action == "campaignQuiet")
        model_.selected_campaign = "A Quiet Beginning";
    else if (action == "campaignOld")
        model_.selected_campaign = "Old Expedition";
    else if (action == "modBase")
        model_.selected_mod = "Base Content";
    else if (action == "modCartographer")
        model_.selected_mod = "Cartographer's Desk";
    else if (action == "modOldLanterns")
        model_.selected_mod = "Old Lanterns";
    else if (action == "modUnderground")
        model_.selected_mod = "Underground Rivers";
    else if (action == "modTempleWeather")
        model_.selected_mod = "Temple Weather";
    else if (action == "modPocket")
        model_.selected_mod = "Pocket Expedition";
    else if (action == "modMycelium")
        model_.selected_mod = "Mycelium Below";
    else if (action == "modBrassline")
        model_.selected_mod = "Brassline Grapple Kit";
    else if (action == "modSkybreak")
        model_.selected_mod = "Skybreak Caverns";
    else if (action == "modAbyssal")
        model_.selected_mod = "Abyssal Tide";
    else if (action == "modMirror")
        model_.selected_mod = "Mirror Depths";
    else if (action == "modClockwork")
        model_.selected_mod = "Clockwork Orchard";
    else if (action == "modLantern")
        model_.selected_mod = "Lantern Cartography";
    else if (action == "modQuietCamps")
        model_.selected_mod = "Quiet Camps";
    else if (action == "modRelic")
        model_.selected_mod = "Relic Ledger";
    else if (action == "bindMenuUp")
        model_.selected_action = "Menu Up";
    else if (action == "bindMenuDown")
        model_.selected_action = "Menu Down";
    else if (action == "bindMenuLeft")
        model_.selected_action = "Menu Left";
    else if (action == "bindMenuRight")
        model_.selected_action = "Menu Right";
    else if (action == "bindActivate")
        model_.selected_action = "Activate";
    else if (action == "bindCancel")
        model_.selected_action = "Cancel";
    else if (action == "bindMove")
        model_.selected_action = "Move";
    else if (action == "bindLook")
        model_.selected_action = "Look";
    else if (action == "bindFire")
        model_.selected_action = "Fire";
    else if (action == "captureBinding")
        model_.capture_binding = !model_.capture_binding;
    else if (action == "toggleReady")
        model_.player_ready = !model_.player_ready;

    refresh_screens();
}

bool TrialHost::switch_screen(std::string_view destination) {
    // activate one retained screen and keep the rest prepared
    const auto found = std::find(kRoutes.begin(), kRoutes.end(), destination);
    if (found == kRoutes.end())
        return false;
    const std::size_t next = static_cast<std::size_t>(found - kRoutes.begin());
    hosts_[active_screen_]->deactivate();
    auto activated = hosts_[next]->activate();
    if (!activated.has_value()) {
        print_diagnostics(activated.diagnostics());
        return false;
    }
    active_screen_ = next;
    model_.destination = std::string{destination};
    refresh_screens();
    return true;
}

void TrialHost::refresh_screens() {
    for (auto& screen : screens_)
        screen->refresh();
}

int TrialHost::run() {
    // init SDL window and GPU presenter
    const auto startup_begin = std::chrono::steady_clock::now();
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        std::fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
        return 1;
    }
    const SDL_WindowFlags flags = static_cast<SDL_WindowFlags>(
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_UTILITY |
        (options_.hidden ? SDL_WINDOW_HIDDEN : 0));
    SDL_Window* window =
        SDL_CreateWindow("Gubsy Arbor trial", options_.width, options_.height, flags);
    if (!window) {
        std::fprintf(stderr, "window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_SetWindowMinimumSize(window, 360, 320);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    auto renderer_result = arbor::sdl3_gpu::GpuRenderer::create(window);
    if (!renderer_result.has_value()) {
        print_diagnostics(renderer_result.diagnostics());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    std::optional<arbor::sdl3_gpu::GpuRenderer> renderer{std::move(renderer_result).value()};

    // open currently attached controllers
    int gamepad_count = 0;
    SDL_JoystickID* gamepads = SDL_GetGamepads(&gamepad_count);
    std::vector<SDL_Gamepad*> opened_gamepads;
    for (int index = 0; index < gamepad_count; ++index) {
        if (SDL_Gamepad* gamepad = SDL_OpenGamepad(gamepads[index])) {
            opened_gamepads.push_back(gamepad);
            if (opened_gamepads.size() == 1) {
                const char* name = SDL_GetGamepadName(gamepad);
                model_.controller_status =
                    name ? std::string{"CONTROLLER - "} + name : "CONTROLLER CONNECTED";
            }
        }
    }
    SDL_free(gamepads);

    // prepare all retained route documents
    const std::filesystem::path ui{GUBSY_ARB_UI_DIR};
    const std::filesystem::path components = ui / "components";
    const std::filesystem::path font{GUBSY_ARB_FONT};
    const auto callback = [this](std::string_view action,
                                 const arbor::widgets::WidgetEvent& event) {
        handle_action(action, event);
    };
    for (std::size_t index = 0; index < kRoutes.size(); ++index) {
        screens_[index] = std::make_unique<TrialScreen>(ui / (std::string{kRoutes[index]} + ".axl"),
                                                        components, font, model_, callback);
        hosts_[index] = std::make_unique<arbor::app::ApplicationHost>(*screens_[index]);
        auto prepared = hosts_[index]->prepare();
        if (!prepared.has_value()) {
            print_diagnostics(prepared.diagnostics());
            for (SDL_Gamepad* gamepad : opened_gamepads)
                SDL_CloseGamepad(gamepad);
            renderer.reset();
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }
    }
    const auto initial = std::find(kRoutes.begin(), kRoutes.end(), options_.screen);
    active_screen_ =
        initial == kRoutes.end() ? 0 : static_cast<std::size_t>(initial - kRoutes.begin());
    model_.destination = std::string{kRoutes[active_screen_]};
    auto activated = hosts_[active_screen_]->activate();
    if (!activated.has_value()) {
        print_diagnostics(activated.diagnostics());
        for (SDL_Gamepad* gamepad : opened_gamepads)
            SDL_CloseGamepad(gamepad);
        renderer.reset();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    refresh_screens();
    samples_.startup_ready_ms = milliseconds(startup_begin, std::chrono::steady_clock::now());

    // run host-owned input and render loop
    bool running = true;
    bool failed = false;
    bool text_input = false;
    std::size_t presented = 0;
    constexpr std::size_t warmup = 120;
    while (running) {
        SDL_Event event{};
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                running = false;
                break;
            }
            if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
                const bool already_open = std::any_of(
                    opened_gamepads.begin(), opened_gamepads.end(), [&](SDL_Gamepad* gamepad) {
                        return SDL_GetGamepadID(gamepad) == event.gdevice.which;
                    });
                if (already_open)
                    continue;
                if (SDL_Gamepad* gamepad = SDL_OpenGamepad(event.gdevice.which)) {
                    opened_gamepads.push_back(gamepad);
                    const char* name = SDL_GetGamepadName(gamepad);
                    model_.controller_status =
                        name ? std::string{"CONTROLLER - "} + name : "CONTROLLER CONNECTED";
                    refresh_screens();
                }
                continue;
            }
            if (event.type == SDL_EVENT_GAMEPAD_REMOVED) {
                const auto removed = std::find_if(
                    opened_gamepads.begin(), opened_gamepads.end(), [&](SDL_Gamepad* gamepad) {
                        return SDL_GetGamepadID(gamepad) == event.gdevice.which;
                    });
                if (removed != opened_gamepads.end()) {
                    SDL_CloseGamepad(*removed);
                    opened_gamepads.erase(removed);
                }
                model_.controller_status =
                    opened_gamepads.empty() ? "KEYBOARD / POINTER" : "CONTROLLER CONNECTED";
                refresh_screens();
                continue;
            }
            if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN &&
                event.gbutton.button == SDL_GAMEPAD_BUTTON_EAST) {
                if (model_.play_view != "lobby") {
                    model_.play_view = "lobby";
                    refresh_screens();
                    continue;
                }
                if (active_screen_ != 0) {
                    if (!switch_screen("play"))
                        running = false, failed = true;
                    continue;
                }
            }
            auto translated = translate_controller_event(event);
            if (!translated.has_value())
                translated = arbor::sdl3_gpu::translate_sdl_event(event);
            if (translated.has_value()) {
                auto handled = hosts_[active_screen_]->handle_input(*translated);
                if (!handled.has_value()) {
                    print_diagnostics(handled.diagnostics());
                    running = false;
                    failed = true;
                    break;
                }
            }
        }
        if (!running)
            break;
        if (pending_screen_.has_value()) {
            if (!switch_screen(*pending_screen_))
                running = false, failed = true;
            pending_screen_.reset();
        }

        // Exercise a visible binding every sampled frame. A static retained tree
        // would otherwise report zero reconciliation and hide update cost.
        if (options_.benchmark_frames && presented >= warmup) {
            const bool alternate = (presented % 2) == 0;
            switch (active_screen_) {
            case 0:
                model_.play_host = alternate ? "Automatic" : "Direct";
                break;
            case 1:
                model_.selected_profile = alternate ? "Moss" : "Vega";
                break;
            case 2:
                model_.brightness = alternate ? 64.0 : 65.0;
                break;
            case 3:
                model_.look_sensitivity = alternate ? 45.0 : 46.0;
                break;
            case 4:
                model_.selected_campaign = alternate ? "The Glass Caverns" : "A Quiet Beginning";
                break;
            case 5:
                model_.selected_mod = alternate ? "Mycelium Below" : "Abyssal Tide";
                break;
            default:
                break;
            }
            screens_[active_screen_]->refresh();
        }

        int width = 0;
        int height = 0;
        int pixel_width = 0;
        int pixel_height = 0;
        SDL_GetWindowSize(window, &width, &height);
        SDL_GetWindowSizeInPixels(window, &pixel_width, &pixel_height);
        const float display_scale = width > 0 && height > 0
                                        ? std::max(static_cast<float>(pixel_width) / width,
                                                   static_cast<float>(pixel_height) / height)
                                        : 1.0F;
        // Preserve the authored 720p game-space proportions on common 1080p
        // output. SDL's independent high-DPI ratio is composed on top.
        const float design_scale = std::clamp(
            std::min(static_cast<float>(width) / 1280.0F, static_cast<float>(height) / 720.0F),
            1.0F, 2.0F);
        const float pixel_scale = display_scale * design_scale;
        const arbor::render::Size logical_size{static_cast<float>(width) / design_scale,
                                               static_cast<float>(height) / design_scale};
        arbor::sdl3_gpu::GpuFrameStats gpu_stats{};
        const auto frame_begin = std::chrono::steady_clock::now();
        auto rendered = hosts_[active_screen_]->render(
            {.logical_size = logical_size, .pixel_scale = pixel_scale},
            [&](const arbor::render::RenderList& list, arbor::render::Size size, float scale) {
                auto result = renderer->render(list, size, scale);
                if (result.has_value())
                    gpu_stats = result.value();
                return result.has_value()
                           ? arbor::Result<void>::success()
                           : arbor::Result<void>::failure(result.diagnostics().front());
            });
        const auto frame_end = std::chrono::steady_clock::now();
        if (!rendered.has_value()) {
            print_diagnostics(rendered.diagnostics());
            running = false;
            failed = true;
            break;
        }

        const bool wants_text = hosts_[active_screen_]->wants_text_input();
        if (wants_text != text_input) {
            wants_text ? SDL_StartTextInput(window) : SDL_StopTextInput(window);
            text_input = wants_text;
        }

        if (options_.benchmark_frames && presented >= warmup) {
            const auto& profile = screens_[active_screen_]->profile();
            samples_.reconcile.push_back(milliseconds(profile.reconcile));
            samples_.style.push_back(milliseconds(profile.style));
            samples_.layout.push_back(milliseconds(profile.layout));
            samples_.render_list.push_back(milliseconds(profile.render_list));
            samples_.ui_total.push_back(milliseconds(profile.total));
            samples_.submit.push_back(milliseconds(gpu_stats.submission_time));
            samples_.frame.push_back(milliseconds(frame_begin, frame_end));
        }
        ++presented;
        if (options_.benchmark_frames && presented >= warmup + options_.benchmark_frames)
            running = false;
    }

    if (options_.benchmark_frames && !failed) {
        for (std::size_t sample = 0; sample < 100; ++sample) {
            const auto hide_begin = std::chrono::steady_clock::now();
            hosts_[active_screen_]->deactivate();
            const auto hide_end = std::chrono::steady_clock::now();
            auto shown = hosts_[active_screen_]->activate();
            const auto show_end = std::chrono::steady_clock::now();
            if (!shown.has_value()) {
                print_diagnostics(shown.diagnostics());
                failed = true;
                break;
            }
            samples_.resident_hide.push_back(milliseconds(hide_begin, hide_end));
            samples_.resident_show.push_back(milliseconds(hide_end, show_end));
        }
    }
    if (options_.benchmark_frames)
        print_benchmark_report(samples_, model_.destination, options_.width, options_.height);
    for (SDL_Gamepad* gamepad : opened_gamepads)
        SDL_CloseGamepad(gamepad);
    renderer.reset();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return failed ? 1 : 0;
}
