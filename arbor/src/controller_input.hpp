#pragma once

#include <SDL3/SDL_events.h>
#include <arbor/input/input.hpp>
#include <optional>

std::optional<arbor::input::InputEvent> translate_controller_event(const SDL_Event& event);
