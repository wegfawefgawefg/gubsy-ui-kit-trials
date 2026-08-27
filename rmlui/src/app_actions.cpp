#include "app.h"

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/SystemInterface.h>

#include <algorithm>
#include <utility>
#include <vector>

// Route mutations and local demo actions.

void GubsyApp::Back() {
  if (!state_.modal.empty()) {
    state_.modal.clear();
    MarkDirty();
    return;
  }
  if (state_.destination == Destination::Play &&
      state_.play_view != PlayView::Lobby) {
    state_.play_view = PlayView::Lobby;
  } else if (state_.destination != Destination::Play) {
    state_.destination = Destination::Play;
    state_.play_view = PlayView::Lobby;
  } else {
    SetToast("Quit confirmation will open here");
    return;
  }
  MarkDirty();
}

void GubsyApp::SetToast(std::string message) {
  toast_ = std::move(message);
  toast_until_ = Rml::GetSystemInterface()->GetElapsedTime() + 2.5;
  if (Rml::Element *toast = document_->GetElementById("toast")) {
    toast->SetInnerRML(toast_);
    toast->SetClass("visible", true);
  }
}

void GubsyApp::HandleAction(const std::string &action) {
  if (action.rfind("nav-", 0) == 0) {
    const std::string name = action.substr(4);
    if (name == "play")
      state_.destination = Destination::Play;
    else if (name == "players")
      state_.destination = Destination::Players;
    else if (name == "settings")
      state_.destination = Destination::Settings;
    else if (name == "controls")
      state_.destination = Destination::Controls;
    else if (name == "progress")
      state_.destination = Destination::Progress;
    else if (name == "mods")
      state_.destination = Destination::Mods;
    state_.play_view = PlayView::Lobby;
    MarkDirty();
    return;
  }
  if (action == "back") {
    Back();
    return;
  }
  if (action == "play-quest")
    state_.play_view = PlayView::Quest;
  else if (action == "play-rules")
    state_.play_view = PlayView::Rules;
  else if (action == "play-mods")
    state_.play_view = PlayView::SessionMods;
  else if (action == "play-lobby")
    state_.play_view = PlayView::Lobby;
  else if (action == "show-setup")
    state_.party_pane = false;
  else if (action == "show-party")
    state_.party_pane = true;
  else if (action == "toggle-shortcuts") {
    state_.selected_rule = "Discovered shortcuts";
    state_.shortcuts = !state_.shortcuts;
  } else if (action == "toggle-treasury") {
    state_.selected_rule = "Shared treasury";
    state_.shared_treasury = !state_.shared_treasury;
  } else if (action == "toggle-friendly-fire") {
    state_.selected_rule = "Friendly fire";
    state_.friendly_fire = !state_.friendly_fire;
  } else if (action == "activity") {
    if (state_.activity == "Continue expedition")
      state_.activity = "New expedition";
    else if (state_.activity == "New expedition")
      state_.activity = "Arena run";
    else
      state_.activity = "Continue expedition";
  } else if (action == "access") {
    if (state_.access == "Friends can join")
      state_.access = "Solo";
    else if (state_.access == "Solo")
      state_.access = "Invite only";
    else if (state_.access == "Invite only")
      state_.access = "Public";
    else
      state_.access = "Friends can join";
  } else if (action == "host") {
    if (state_.host == "Automatic")
      state_.host = "Host locally";
    else if (state_.host == "Host locally")
      state_.host = "Dedicated relay";
    else
      state_.host = "Automatic";
  } else if (action == "quest-violet")
    state_.selected_quest = "The Violet Reach";
  else if (action == "quest-sunken")
    state_.selected_quest = "The Sunken Road";
  else if (action == "quest-green")
    state_.selected_quest = "A Green Beginning";
  else if (action == "quest-glass")
    state_.selected_quest = "The Glass Pilgrim";
  else if (action == "profile-moss")
    state_.selected_profile = "Moss";
  else if (action == "profile-vega")
    state_.selected_profile = "Vega";
  else if (action == "profile-guest")
    state_.selected_profile = "Guest";
  else if (action == "device-keyboard")
    state_.selected_device = "Keyboard + Mouse";
  else if (action == "device-xbox")
    state_.selected_device = "Xbox Wireless Controller";
  else if (action == "device-dualsense")
    state_.selected_device = "DualSense Wireless Controller";
  else if (action == "device-flight")
    state_.selected_device = "T.16000M Flight Stick";
  else if (action == "device-pedals")
    state_.selected_device = "T-LCM Pedals";
  else if (action == "mod-base")
    state_.selected_mod = "Base Content";
  else if (action == "mod-lanterns")
    state_.selected_mod = "Old Lanterns";
  else if (action == "mod-mycelium")
    state_.selected_mod = "Mycelium Below";
  else if (action == "mod-grapple")
    state_.selected_mod = "Brassline Grapple Kit";
  else if (action == "mod-skybreak")
    state_.selected_mod = "Skybreak Caverns";
  else if (action == "mod-tide")
    state_.selected_mod = "Abyssal Tide";
  else if (action == "mod-depths")
    state_.selected_mod = "Mirror Depths";
  else if (action.rfind("installed-mod-", 0) == 0)
    state_.selected_mod = action.substr(14);
  else if (action.rfind("select-rule-", 0) == 0)
    state_.selected_rule = action.substr(12);
  else if (action.rfind("action-", 0) == 0)
    state_.selected_action = action.substr(7);
  else if (action.rfind("campaign-", 0) == 0)
    state_.selected_campaign = action.substr(9);
  else if (action.rfind("setting-", 0) == 0)
    state_.selected_setting = action.substr(8);
  else if (action == "toggle-compatible")
    state_.compatible_only = !state_.compatible_only;
  else if (action == "session-current")
    state_.session_mod_browse = false;
  else if (action == "session-browse")
    state_.session_mod_browse = true;
  else if (action.rfind("session-mod-", 0) == 0)
    state_.selected_mod = action.substr(12);
  else if (action.rfind("catalog-mod-", 0) == 0)
    state_.selected_mod = action.substr(12);
  else if (action == "install-add-session") {
    state_.session_mod_browse = false;
    SetToast("Installed dependencies and added " + state_.selected_mod +
             " to this session");
  } else if (action == "toggle-ready")
    state_.player_ready = !state_.player_ready;
  else if (action == "capture-binding") {
    state_.capture_mode = !state_.capture_mode;
    SetToast(state_.capture_mode
                 ? "Listening: actuate any control, or choose explicitly"
                 : "Input capture cancelled");
  } else if (action == "choose-binding") {
    if (Rml::Element *device = document_->QuerySelector(
            "[data-action='choice-value-binding-device']")) {
      device->Focus(true);
      device->ScrollIntoView(false);
    }
    SetToast("Explicit binding editor focused");
    return;
  } else if (action == "uninstall-plan" || action == "delete-campaign" ||
             action == "delete-binds" || action == "quit") {
    state_.modal = action;
  } else if (action == "cancel-modal")
    state_.modal.clear();
  else if (action == "retry-provider" || action == "create-first")
    state_.provider_state = "Populated";
  else if (action == "confirm-modal") {
    state_.modal.clear();
    SetToast("Confirmed local demo mutation");
  } else if (action == "start-session") {
    state_.session_running = true;
    SetToast("Session started from native lobby state");
  } else if (action.rfind("player-tab-", 0) == 0) {
    state_.player_tab = action.substr(11);
  } else if (action.rfind("settings-tab-", 0) == 0) {
    state_.settings_tab = action.substr(13);
    if (state_.settings_tab == "Display")
      state_.selected_setting = "Fullscreen";
    else if (state_.settings_tab == "Audio")
      state_.selected_setting = "Master volume";
    else if (state_.settings_tab == "Accessibility")
      state_.selected_setting = "Text scale";
    else
      state_.selected_setting = "Context tutorials";
  } else if (action.rfind("controls-tab-", 0) == 0) {
    state_.controls_tab = action.substr(13);
  } else if (action.rfind("mods-tab-", 0) == 0) {
    state_.mods_tab = action.substr(9);
    state_.selected_mod =
        state_.mods_tab == "Browse catalog" ? "Mycelium Below" : "Old Lanterns";
  } else if (action == "local-tab-prev" || action == "local-tab-next") {
    const int step = action == "local-tab-next" ? 1 : -1;
    auto cycle = [step](std::string &value,
                        const std::vector<std::string> &choices) {
      auto it = std::find(choices.begin(), choices.end(), value);
      int index =
          it == choices.end() ? 0 : static_cast<int>(it - choices.begin());
      index = (index + step + static_cast<int>(choices.size())) %
              static_cast<int>(choices.size());
      value = choices[index];
    };
    if (state_.destination == Destination::Play &&
        state_.play_view == PlayView::SessionMods) {
      state_.session_mod_browse = step > 0;
      pending_focus_action_ =
          state_.session_mod_browse ? "session-browse" : "session-current";
    } else if (state_.destination == Destination::Play &&
               state_.play_view == PlayView::Lobby &&
               (viewport_width_ <= 700 ||
                (viewport_height_ <= 500 && viewport_width_ <= 1000))) {
      state_.party_pane = step > 0;
      pending_focus_action_ = state_.party_pane ? "show-party" : "show-setup";
    } else if (state_.destination == Destination::Players) {
      cycle(state_.player_tab, {"Local players", "Profiles", "Devices"});
      pending_focus_action_ = "player-tab-" + state_.player_tab;
    } else if (state_.destination == Destination::Settings) {
      cycle(state_.settings_tab,
            {"Display", "Audio", "Accessibility", "Gameplay"});
      if (state_.settings_tab == "Display")
        state_.selected_setting = "Fullscreen";
      else if (state_.settings_tab == "Audio")
        state_.selected_setting = "Master volume";
      else if (state_.settings_tab == "Accessibility")
        state_.selected_setting = "Text scale";
      else
        state_.selected_setting = "Context tutorials";
      pending_focus_action_ = "settings-tab-" + state_.settings_tab;
    } else if (state_.destination == Destination::Controls) {
      cycle(state_.controls_tab, {"Bindings", "Devices", "Input tuning"});
      pending_focus_action_ = "controls-tab-" + state_.controls_tab;
    } else if (state_.destination == Destination::Mods) {
      cycle(state_.mods_tab, {"Installed", "Browse catalog"});
      pending_focus_action_ = "mods-tab-" + state_.mods_tab;
    } else {
      SetToast("No local tabs on this screen");
      return;
    }
  } else {
    SetToast("Action: " + action);
    return;
  }
  MarkDirty();
}
