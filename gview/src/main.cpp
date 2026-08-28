#include "app.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <vector>

TrialOptions parse_options(int argc, char** argv) {
    TrialOptions options;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        const auto integer = [&](int& target) {
            if (index + 1 < argc) target = std::atoi(argv[++index]);
        };
        if (argument == "--width") integer(options.width);
        else if (argument == "--height") integer(options.height);
        else if (argument == "--screen") integer(options.screen);
        else if (argument == "--frames") integer(options.frames);
        else if (argument == "--hidden") options.hidden = true;
        else if (argument == "--benchmark") options.benchmark = true;
        else if (argument == "--self-test") options.self_test = true;
        else if (argument == "--capture" && index + 1 < argc) options.capture = argv[++index];
    }
    return options;
}

// Centers the utility window on the leftmost display before it is shown.
void position_window_on_left_display(SDL_Window* window, int width, int height) {
    int count = 0;
    SDL_DisplayID* displays = SDL_GetDisplays(&count);
    SDL_Rect chosen{};
    bool found = false;
    for (int index = 0; index < count; ++index) {
        SDL_Rect bounds{};
        if (SDL_GetDisplayUsableBounds(displays[index], &bounds) && (!found || bounds.x < chosen.x)) {
            chosen = bounds;
            found = true;
        }
    }
    SDL_free(displays);
    if (found)
        SDL_SetWindowPosition(window, chosen.x + (chosen.w - width) / 2,
                             chosen.y + (chosen.h - height) / 2);
}

int main(int argc, char** argv) {
    const TrialOptions options = parse_options(argc, argv);
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY |
                            SDL_WINDOW_UTILITY | SDL_WINDOW_HIDDEN;
    SDL_Window* window = SDL_CreateWindow("Gubsy GView trial", options.width, options.height, flags);
    if (!window) return 1;
    position_window_on_left_display(window, options.width, options.height);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::fprintf(stderr, "SDL renderer failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetRenderVSync(renderer, options.benchmark ? 0 : 1);
    SDL_SetRenderLogicalPresentation(renderer, options.width, options.height,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
    SDL_StartTextInput(window);
    {
    TrialApp app(renderer, options.width, options.height);
    app.select_screen(options.screen);
    if (!app.ready()) return 1;
    if (options.self_test) return run_self_test(app) ? 0 : 2;
    if (!options.hidden) SDL_ShowWindow(window);

    std::vector<double> updates;
    std::vector<double> renders;
    int frames = 0;
    while (app.running()) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) app.process(event);
        app.update();
        app.render();
        updates.push_back(app.update_ms());
        renders.push_back(app.render_ms());
        ++frames;
        if (!options.capture.empty() && frames == 3) {
            SDL_Surface* capture = SDL_RenderReadPixels(renderer, nullptr);
            if (capture) {
                SDL_SaveBMP(capture, options.capture.c_str());
                SDL_DestroySurface(capture);
            }
        }
        if (options.frames > 0 && frames >= options.frames) break;
        if (!options.capture.empty() && frames >= 3) break;
        if (options.benchmark && frames >= 2000) break;
    }
    if (options.benchmark) {
        const auto report = [](const char* name, std::vector<double> samples) {
            std::sort(samples.begin(), samples.end());
            double total = 0.0;
            for (double value : samples) total += value;
            const auto percentile = [&](double fraction) {
                const double last = static_cast<double>(samples.size() - 1);
                return samples[static_cast<std::size_t>(last * fraction)];
            };
            std::printf("%s mean=%.4fms p95=%.4fms p99=%.4fms max=%.4fms\n", name,
                        total / static_cast<double>(samples.size()), percentile(0.95),
                        percentile(0.99), samples.back());
        };
        report("update", updates);
        report("render", renders);
        std::printf("layout_builds=%llu paint_builds=%llu frames=%llu\n",
                    static_cast<unsigned long long>(app.stats().layout_builds),
                    static_cast<unsigned long long>(app.stats().paint_builds),
                    static_cast<unsigned long long>(app.stats().frames));
    }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
