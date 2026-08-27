#pragma once

#include <string>
#include <unordered_map>

enum class Destination { Play, Players, Settings, Controls, Progress, Mods };
enum class PlayView { Lobby, Quest, Rules, SessionMods };

struct UiState {
  Destination destination = Destination::Play;
  PlayView play_view = PlayView::Lobby;

  std::string player_tab = "Local players";
  std::string settings_tab = "Display";
  std::string controls_tab = "Bindings";
  std::string mods_tab = "Installed";

  std::string activity = "Continue expedition";
  std::string access = "Friends can join";
  std::string host = "Automatic";
  std::string selected_quest = "The Violet Reach";
  std::string selected_checkpoint = "Temple safe room";
  std::string selected_rule = "Discovered shortcuts";
  std::string selected_mod = "Old Lanterns";
  std::string selected_profile = "Moss";
  std::string selected_device = "Keyboard + Mouse";
  std::string selected_action = "Menu Up";
  std::string selected_campaign = "The Glass Caverns";
  std::string selected_setting = "Fullscreen";
  std::string provider_state = "Populated";
  std::string control_filter;
  std::string mod_filter;
  std::string modal;

  std::unordered_map<std::string, std::string> setting_values{
      {"fullscreen", "true"},      {"resolution", "1920 × 1080"},
      {"render-scale", "100"},     {"frame-cap", "144 FPS"},
      {"brightness", "64"},        {"vsync", "Adaptive"},
      {"master-volume", "82"},     {"music-volume", "64"},
      {"effects-volume", "90"},    {"dialogue-volume", "100"},
      {"dynamic-range", "Night"},  {"output-device", "Default"},
      {"text-scale", "125%"},      {"high-contrast", "true"},
      {"reduce-motion", "false"},  {"color-filter", "Deuteranopia"},
      {"hold-assists", "true"},    {"narration", "false"},
      {"tutorials", "Smart"},      {"camera-shake", "35"},
      {"auto-pause", "true"},      {"checkpoint-hints", "true"},
      {"speedrun-timer", "false"}, {"telemetry", "false"}};
  std::unordered_map<std::string, std::string> rule_values{
      {"difficulty", "Standard"},  {"variation", "Quest-authored"},
      {"shared-lives", "4"},       {"starting-health", "4"},
      {"ghost-arrival", "180"},    {"shop-frequency", "Normal"},
      {"lantern-fuel", "Standard"}};
  std::unordered_map<std::string, std::string> tuning_values{
      {"look-sensitivity", "45"},   {"stick-deadzone", "12"},
      {"vibration", "80"},          {"trigger-deadzone", "5"},
      {"response-curve", "Smooth"}, {"invert-y", "false"}};
  std::unordered_map<std::string, std::string> choice_values{
      {"player-profile", "Moss"},
      {"player-bindings", "Default Binds"},
      {"player-tuning", "Standard"},
      {"bind-profile", "Default Binds"},
      {"binding-device", "Xbox Wireless Controller"},
      {"binding-control", "Right Trigger"},
      {"binding-conversion", "Axis → Button"},
      {"binding-threshold", "0.62 · Rising"},
      {"mod-version", "v1.4.0"}};

  bool party_pane = false;
  bool session_running = false;
  bool shortcuts = true;
  bool shared_treasury = true;
  bool friendly_fire = false;
  bool compatible_only = false;
  bool session_mod_browse = false;
  bool capture_mode = false;
  bool player_ready = true;

  int shared_lives = 4;
  int health = 4;
  int ghost_seconds = 180;
  float raw_input_value = 0.742f;
  std::string raw_input_name = "Axis 4 · Right Trigger";
};
