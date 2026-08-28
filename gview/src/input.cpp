#include "app.hpp"

#include <algorithm>
#include <imgui.h>

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
    } else if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_F3 && !event.key.repeat) {
        authoring_enabled_ = !authoring_enabled_;
        return;
    }
    if (authoring_enabled_ &&
        ((ImGui::GetIO().WantCaptureKeyboard && event.type == SDL_EVENT_KEY_DOWN) ||
         (ImGui::GetIO().WantCaptureMouse && event.type >= SDL_EVENT_MOUSE_MOTION &&
          event.type <= SDL_EVENT_MOUSE_WHEEL)))
        return;
    if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
        resize(event.window.data1, event.window.data2);
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
        if (event.key.key == SDLK_BACKSPACE) input_.text.push_back('\b');
        else if (event.key.key == SDLK_UP) input_.navigation.push_back(gview::NavAction::Up);
        else if (event.key.key == SDLK_DOWN) input_.navigation.push_back(gview::NavAction::Down);
        else if (event.key.key == SDLK_LEFT) input_.navigation.push_back(gview::NavAction::Left);
        else if (event.key.key == SDLK_RIGHT) input_.navigation.push_back(gview::NavAction::Right);
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
