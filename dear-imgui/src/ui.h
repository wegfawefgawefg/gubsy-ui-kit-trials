#pragma once

#include <array>
#include "imgui.h"

struct UiAssets {
  ImTextureID mod_sheet = ImTextureID_Invalid;
};

struct UiState {
  int screen = 0;
  int play_activity = 0;
  int play_notice = 0;
  int play_view = 0;
  int selected_checkpoint = 0;
  int session_mods_tab = 0;
  int selected_session_mod = 0;
  std::array<bool,8> session_enabled{true,true,true,true,true,true,true,false};
  std::array<bool,20> catalog_installed{};
  int host_mode = 0;
  int join_mode = 1;
  int players_tab = 0;
  int settings_tab = 0;
  int controls_tab = 0;
  int controls_view = 0;
  int browsed_input = 0;
  int mods_tab = 1;
  int selected_action = 0;
  int selected_profile = 0;
  int selected_device = 0;
  int selected_progress = 0;
  int selected_mod = 0;
  int selected_installed = 0;
  bool fullscreen = false;
  bool subtitles = true;
  bool color_distinction = true;
  bool reduced_motion = false;
  bool pause_unfocused = true;
  bool tutorial_prompts = true;
  bool damage_numbers = false;
  bool local_ready = true;
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
  float shared_lives = 4.0f;
  float starting_health = 4.0f;
  float ghost_arrival = 180.0f;
  bool discovered_shortcuts = true;
  int expedition_difficulty = 1;
  int stage_variation = 0;
  int shop_frequency = 1;
  int revive_mode = 0;
  float enemy_damage = 100.0f;
  bool shared_treasury = true;
};

void ConfigureUiStyle(float density);
void SetUiAssets(UiAssets assets);
void DrawGubsyUi(UiState& state, int width, int height);
