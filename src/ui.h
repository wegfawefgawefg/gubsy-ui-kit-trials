#pragma once

#include <array>

struct UiState {
  int screen = 0;
  int play_activity = 0;
  int host_mode = 0;
  int join_mode = 1;
  int players_tab = 0;
  int settings_tab = 0;
  int controls_tab = 0;
  int mods_tab = 1;
  int selected_row = 0;
  bool fullscreen = false;
  bool subtitles = true;
  bool friendly_fire = false;
  bool compatible_only = false;
  float render_scale = 100.0f;
  float brightness = 64.0f;
  float master_volume = 80.0f;
  float music_volume = 72.0f;
  float sensitivity = 45.0f;
  float deadzone = 12.0f;
  float vibration = 80.0f;
  float trigger_deadzone = 5.0f;
};

void ConfigureUiStyle(float density);
void DrawGubsyUi(UiState& state, int width, int height);
