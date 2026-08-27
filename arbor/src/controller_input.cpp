#include "controller_input.hpp"

#include <SDL3/SDL_gamepad.h>

std::optional<arbor::input::InputEvent> translate_controller_event(const SDL_Event& event) {
    // Translate SDL gamepads into Arbor's native retained-widget input model.
    if (event.type != SDL_EVENT_GAMEPAD_BUTTON_DOWN && event.type != SDL_EVENT_GAMEPAD_BUTTON_UP)
        return std::nullopt;

    const bool down = event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN;
    arbor::input::Key key = arbor::input::Key::unknown;
    arbor::input::KeyModifiers modifiers{};
    switch (event.gbutton.button) {
    case SDL_GAMEPAD_BUTTON_SOUTH:
    case SDL_GAMEPAD_BUTTON_START:
        key = arbor::input::Key::enter;
        break;
    case SDL_GAMEPAD_BUTTON_EAST:
        key = arbor::input::Key::escape;
        break;
    case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
        key = arbor::input::Key::tab;
        break;
    case SDL_GAMEPAD_BUTTON_DPAD_UP:
        key = arbor::input::Key::tab;
        modifiers.shift = true;
        break;
    case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
    case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
        key = arbor::input::Key::left;
        break;
    case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
    case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
        key = arbor::input::Key::right;
        break;
    default:
        return std::nullopt;
    }

    if (down)
        return arbor::input::InputEvent{arbor::input::KeyDown{key, false, modifiers}};
    return arbor::input::InputEvent{arbor::input::KeyUp{key, modifiers}};
}
