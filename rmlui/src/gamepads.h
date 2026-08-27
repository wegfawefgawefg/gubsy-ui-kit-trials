#pragma once

#include <SDL3/SDL.h>

#include <vector>

class GubsyApp;

using OpenGamepads = std::vector<SDL_Gamepad *>;

void open_connected_gamepads(OpenGamepads &gamepads, GubsyApp &app);
void handle_gamepad_connection(const SDL_Event &event, OpenGamepads &gamepads,
                               GubsyApp &app);
void close_gamepads(OpenGamepads &gamepads);
