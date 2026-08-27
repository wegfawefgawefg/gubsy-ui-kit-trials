#include "gamepads.h"

#include "app.h"

#include <algorithm>
#include <cstdio>

namespace {

void report_gamepad_status(const OpenGamepads &gamepads, GubsyApp &app) {
  // report first active controller to shell
  const char *name =
      gamepads.empty() ? "" : SDL_GetGamepadName(gamepads.front());
  app.SetGamepadStatus(static_cast<int>(gamepads.size()),
                       name ? name : "Gamepad");
}

bool has_gamepad(const OpenGamepads &gamepads, SDL_JoystickID id) {
  return std::any_of(gamepads.begin(), gamepads.end(),
                     [id](SDL_Gamepad *gamepad) {
                       return SDL_GetGamepadID(gamepad) == id;
                     });
}

void open_gamepad(SDL_JoystickID id, OpenGamepads &gamepads) {
  // open one untracked controller
  if (has_gamepad(gamepads, id))
    return;
  if (SDL_Gamepad *gamepad = SDL_OpenGamepad(id)) {
    gamepads.push_back(gamepad);
    std::fprintf(stderr, "Opened gamepad %u: %s (%s)\n",
                 SDL_GetGamepadID(gamepad), SDL_GetGamepadName(gamepad),
                 SDL_GetGamepadPath(gamepad));
  } else {
    std::fprintf(stderr, "Could not open gamepad %u: %s\n", id,
                 SDL_GetError());
  }
}

} // namespace

void open_connected_gamepads(OpenGamepads &gamepads, GubsyApp &app) {
  // open controllers present at startup
  int count = 0;
  if (SDL_JoystickID *ids = SDL_GetGamepads(&count)) {
    for (int index = 0; index < count; ++index)
      open_gamepad(ids[index], gamepads);
    SDL_free(ids);
  }
  report_gamepad_status(gamepads, app);
}

void handle_gamepad_connection(const SDL_Event &event, OpenGamepads &gamepads,
                               GubsyApp &app) {
  // track controller hotplug lifetime
  if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
    open_gamepad(event.gdevice.which, gamepads);
    report_gamepad_status(gamepads, app);
    return;
  }
  if (event.type != SDL_EVENT_GAMEPAD_REMOVED)
    return;
  const auto found =
      std::find_if(gamepads.begin(), gamepads.end(), [&](SDL_Gamepad *gamepad) {
        return SDL_GetGamepadID(gamepad) == event.gdevice.which;
      });
  if (found == gamepads.end())
    return;
  SDL_CloseGamepad(*found);
  gamepads.erase(found);
  report_gamepad_status(gamepads, app);
}

void close_gamepads(OpenGamepads &gamepads) {
  // close owned controller handles
  for (SDL_Gamepad *gamepad : gamepads)
    SDL_CloseGamepad(gamepad);
  gamepads.clear();
}
