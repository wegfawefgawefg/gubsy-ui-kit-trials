#include "app.hpp"

#include <algorithm>
#include <imgui.h>

namespace {

bool directional(gview::NavAction action) {
    return action == gview::NavAction::Up || action == gview::NavAction::Down ||
           action == gview::NavAction::Left || action == gview::NavAction::Right;
}

} // namespace

// Converts SDL devices to semantic pointer and navigation input.
void TrialApp::process(const SDL_Event& source) {
    SDL_Event event = source;
    SDL_ConvertEventToRenderCoordinates(renderer_, &event);
    if (event.type == SDL_EVENT_QUIT) running_ = false;
    else if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
        open_gamepad(event.gdevice.which);
        return;
    } else if (event.type == SDL_EVENT_GAMEPAD_REMOVED) {
        close_gamepad(event.gdevice.which);
        return;
    } else if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat &&
               event.key.key >= SDLK_F1 && event.key.key <= SDLK_F5) {
        if (event.key.key == SDLK_F1) {
            set_authoring_enabled(!authoring_enabled_);
            if (authoring_enabled_) authoring_ui_.show_launcher = true;
        } else {
            set_authoring_enabled(true);
            if (event.key.key == SDLK_F2)
                authoring_ui_.mode = authoring_ui_.mode == gview::AuthoringMode::Test
                                         ? gview::AuthoringMode::Edit
                                         : gview::AuthoringMode::Test;
            else if (event.key.key == SDLK_F3)
                authoring_ui_.show_layout_boxes = !authoring_ui_.show_layout_boxes;
            else if (event.key.key == SDLK_F4)
                authoring_ui_.show_grid = !authoring_ui_.show_grid;
            else if (event.key.key == SDLK_F5)
                authoring_ui_.show_focus_overlay = !authoring_ui_.show_focus_overlay;
        }
        return;
    }
    if (authoring_enabled_ && gview::authoring_captures_runtime(authoring_ui_) &&
        event.type != SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
        return;
    if (authoring_enabled_ &&
        ((ImGui::GetIO().WantCaptureKeyboard && event.type == SDL_EVENT_KEY_DOWN) ||
         (ImGui::GetIO().WantCaptureMouse && event.type >= SDL_EVENT_MOUSE_MOTION &&
          event.type <= SDL_EVENT_MOUSE_WHEEL)))
        return;
    if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) return;
    else if (event.type == SDL_EVENT_MOUSE_MOTION) {
        input_.pointer.x = event.motion.x;
        input_.pointer.y = event.motion.y;
        input_.pointer.moved = true;
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
               event.button.button == SDL_BUTTON_LEFT) {
        input_.pointer.x = event.button.x;
        input_.pointer.y = event.button.y;
        input_.pointer.pressed = true;
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT) {
        input_.pointer.x = event.button.x;
        input_.pointer.y = event.button.y;
        input_.pointer.released = true;
    } else if (event.type == SDL_EVENT_MOUSE_WHEEL) {
        input_.pointer.x = event.wheel.mouse_x;
        input_.pointer.y = event.wheel.mouse_y;
        input_.pointer.scroll_y += event.wheel.y;
    } else if (event.type == SDL_EVENT_TEXT_INPUT) {
        input_.text += event.text.text;
    } else if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
        const gview::NodeIndex focus = runtime_.focus();
        const bool editing_text =
            focus != gview::invalid_node && focus < runtime_.state().size() &&
            runtime_.state()[focus].editing &&
            runtime_.view().nodes[focus].source.control == gview::ControlKind::TextInput;
        if (event.key.key == SDLK_BACKSPACE) input_.text.push_back('\b');
        else if (event.key.key == SDLK_UP || (!editing_text && event.key.key == SDLK_W))
            input_.navigation.push_back(gview::NavAction::Up);
        else if (event.key.key == SDLK_DOWN || (!editing_text && event.key.key == SDLK_S))
            input_.navigation.push_back(gview::NavAction::Down);
        else if (event.key.key == SDLK_LEFT || (!editing_text && event.key.key == SDLK_A))
            input_.navigation.push_back(gview::NavAction::Left);
        else if (event.key.key == SDLK_RIGHT || (!editing_text && event.key.key == SDLK_D))
            input_.navigation.push_back(gview::NavAction::Right);
        else if (event.key.key == SDLK_RETURN || event.key.key == SDLK_SPACE)
            input_.navigation.push_back(gview::NavAction::Confirm);
        else if (event.key.key == SDLK_ESCAPE) input_.navigation.push_back(gview::NavAction::Back);
        else if (event.key.key == SDLK_Q)
            input_.navigation.push_back(gview::NavAction::TabPrevious);
        else if (event.key.key == SDLK_E) input_.navigation.push_back(gview::NavAction::TabNext);
    } else if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
        if (event.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_UP)
            input_.navigation.push_back(gview::NavAction::Up);
        else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_DOWN)
            input_.navigation.push_back(gview::NavAction::Down);
        else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_LEFT)
            input_.navigation.push_back(gview::NavAction::Left);
        else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_RIGHT)
            input_.navigation.push_back(gview::NavAction::Right);
        else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH)
            input_.navigation.push_back(gview::NavAction::Confirm);
        else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_EAST)
            input_.navigation.push_back(gview::NavAction::Back);
        else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_LEFT_SHOULDER)
            input_.navigation.push_back(gview::NavAction::TabPrevious);
        else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER)
            input_.navigation.push_back(gview::NavAction::TabNext);
    } else if (event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
        constexpr int threshold = 18000;
        const int direction = event.gaxis.value > threshold    ? 1
                              : event.gaxis.value < -threshold ? -1
                                                               : 0;
        if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX) {
            if (direction != 0 && direction != axis_x_)
                input_.navigation.push_back(direction < 0 ? gview::NavAction::Left
                                                          : gview::NavAction::Right);
            axis_x_ = direction;
        } else if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTY) {
            if (direction != 0 && direction != axis_y_)
                input_.navigation.push_back(direction < 0 ? gview::NavAction::Up
                                                          : gview::NavAction::Down);
            axis_y_ = direction;
        }
    }
}

// Reports one currently held semantic direction across keyboard and every
// connected gamepad; binding translation can replace this polling later.
std::optional<gview::NavAction> TrialApp::held_navigation() const {
    const gview::NodeIndex focus = runtime_.focus();
    const bool editing_text =
        focus != gview::invalid_node && focus < runtime_.state().size() &&
        runtime_.state()[focus].editing &&
        runtime_.view().nodes[focus].source.control == gview::ControlKind::TextInput;
    const bool* keys = SDL_GetKeyboardState(nullptr);
    int horizontal = 0;
    int vertical = 0;
    if (keys[SDL_SCANCODE_LEFT] || (!editing_text && keys[SDL_SCANCODE_A])) --horizontal;
    if (keys[SDL_SCANCODE_RIGHT] || (!editing_text && keys[SDL_SCANCODE_D])) ++horizontal;
    if (keys[SDL_SCANCODE_UP] || (!editing_text && keys[SDL_SCANCODE_W])) --vertical;
    if (keys[SDL_SCANCODE_DOWN] || (!editing_text && keys[SDL_SCANCODE_S])) ++vertical;

    constexpr Sint16 threshold = 18000;
    for (SDL_Gamepad* gamepad : gamepads_) {
        if (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT)) --horizontal;
        if (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT)) ++horizontal;
        if (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP)) --vertical;
        if (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN)) ++vertical;
        const Sint16 x = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX);
        const Sint16 y = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY);
        if (x < -threshold) --horizontal;
        else if (x > threshold) ++horizontal;
        if (y < -threshold) --vertical;
        else if (y > threshold) ++vertical;
    }
    if (vertical < 0) return gview::NavAction::Up;
    if (vertical > 0) return gview::NavAction::Down;
    if (horizontal < 0) return gview::NavAction::Left;
    if (horizontal > 0) return gview::NavAction::Right;
    return std::nullopt;
}

// Emits controlled held-direction repeats independently of desktop keyboard
// repeat settings and with identical timing for D-pad and analog navigation.
void TrialApp::update_navigation_repeat() {
    constexpr Uint64 initial_delay = 360;
    constexpr Uint64 repeat_interval = 85;
    if ((authoring_enabled_ && gview::authoring_captures_runtime(authoring_ui_)) ||
        (authoring_enabled_ && ImGui::GetIO().WantCaptureKeyboard)) {
        repeated_navigation_.reset();
        return;
    }
    const std::optional<gview::NavAction> held = held_navigation();
    if (!held) {
        repeated_navigation_.reset();
        return;
    }
    const Uint64 now = SDL_GetTicks();
    if (held != repeated_navigation_) {
        const bool initial_event_present =
            std::find_if(input_.navigation.begin(), input_.navigation.end(), [&](const auto action) {
                return action == *held && directional(action);
            }) != input_.navigation.end();
        if (!initial_event_present) input_.navigation.push_back(*held);
        repeated_navigation_ = held;
        next_navigation_repeat_ = now + initial_delay;
        return;
    }
    if (now < next_navigation_repeat_) return;
    input_.navigation.push_back(*held);
    next_navigation_repeat_ = now + repeat_interval;
}
