#include "app.h"
#include "focus_elements.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Elements/ElementFormControlSelect.h>

#include <string>

// Directional focus, local memory, and transactional controls.

bool GubsyApp::RunNavigationSelfTest() {
  SDL_Event activate{};
  activate.type = SDL_EVENT_GAMEPAD_BUTTON_DOWN;
  activate.gbutton.button = SDL_GAMEPAD_BUTTON_SOUTH;
  SDL_Event adjust_range{};
  adjust_range.type = SDL_EVENT_GAMEPAD_BUTTON_DOWN;
  adjust_range.gbutton.button = SDL_GAMEPAD_BUTTON_DPAD_RIGHT;
  SDL_Event choose_next{};
  choose_next.type = SDL_EVENT_GAMEPAD_BUTTON_DOWN;
  choose_next.gbutton.button = SDL_GAMEPAD_BUTTON_DPAD_DOWN;

  // Lobby relationships stay within their local pane.
  SelectToolScreen(0);
  Update();
  context_->Update();
  Rml::Element *activity_control =
      document_->QuerySelector("[data-action='play-value-activity']");
  if (!activity_control)
    return false;
  activity_control->Focus(true);
  constexpr const char *lobby_down_sequence[]{
      "play-quest", "play-value-access", "play-value-host", "play-rules",
      "play-mods"};
  for (const char *expected : lobby_down_sequence) {
    NavigateFocus(0, 1);
    if (!context_->GetFocusElement() ||
        context_->GetFocusElement()->GetAttribute<Rml::String>(
            "data-action", "") != expected)
      return false;
  }
  Rml::Element *session_mods = context_->GetFocusElement();
  NavigateFocus(1, 0);
  if (context_->GetFocusElement() != session_mods)
    return false;
  NavigateFocus(-1, 0);
  if (context_->GetFocusElement() != session_mods)
    return false;

  Rml::Element *resume =
      document_->QuerySelector("[data-action='start-session']");
  Rml::Element *pause =
      document_->QuerySelector("[data-action='pause-preview']");
  if (!resume || !pause)
    return false;
  resume->Focus(true);
  NavigateFocus(-1, 0);
  if (context_->GetFocusElement() != pause)
    return false;

  Rml::Element *open_slot =
      document_->QuerySelector("[data-action='add-player']");
  if (!open_slot)
    return false;
  open_slot->Focus(true);
  NavigateFocus(1, 0);
  if (context_->GetFocusElement() != open_slot)
    return false;

  Rml::Element *host_control =
      document_->QuerySelector("[data-action='play-value-host']");
  host_control->Focus(true);
  SDL_Event controller_back{};
  controller_back.type = SDL_EVENT_GAMEPAD_BUTTON_DOWN;
  controller_back.gbutton.button = SDL_GAMEPAD_BUTTON_EAST;
  if (!HandleSdlEvent(controller_back) ||
      context_->GetFocusElement() != document_->GetElementById("nav-play"))
    return false;
  if (!HandleSdlEvent(activate) || context_->GetFocusElement() != host_control)
    return false;
  if (!HandleSdlEvent(controller_back) ||
      context_->GetFocusElement() != document_->GetElementById("nav-play"))
    return false;

  // Range edits are reversible with controller back.
  SelectToolScreen(2);
  Update();
  context_->Update();
  Rml::Element *shared_lives =
      document_->QuerySelector("[data-action='rule-value-shared-lives']");
  if (!shared_lives)
    return false;
  shared_lives->Focus(true);
  NavigateFocus(0, 1);
  if (!context_->GetFocusElement() ||
      context_->GetFocusElement()->GetAttribute<Rml::String>(
          "data-action", "") != "rule-value-starting-health")
    return false;
  NavigateFocus(0, -1);
  if (context_->GetFocusElement() != shared_lives)
    return false;
  const std::string lives_before_cancel =
      state_.rule_values["shared-lives"];
  if (!HandleSdlEvent(activate) || !controller_editing_ ||
      !HandleSdlEvent(adjust_range) ||
      state_.rule_values["shared-lives"] == lives_before_cancel ||
      !HandleSdlEvent(controller_back) || controller_editing_ ||
      state_.rule_values["shared-lives"] != lives_before_cancel)
    return false;

  // Primary navigation remembers local content focus.
  SelectToolScreen(0);
  Update();
  context_->Update();
  Rml::Element *nav_play = document_->GetElementById("nav-play");
  if (!nav_play)
    return false;
  nav_play->Focus(true);
  SDL_Event primary_navigation{};
  primary_navigation.type = SDL_EVENT_GAMEPAD_BUTTON_DOWN;
  primary_navigation.gbutton.button = SDL_GAMEPAD_BUTTON_DPAD_DOWN;
  if (!HandleSdlEvent(primary_navigation) ||
      state_.destination != Destination::Players ||
      context_->GetFocusElement() != document_->GetElementById("nav-players"))
    return false;
  Update();
  context_->Update();
  if (!HandleSdlEvent(activate) ||
      context_->GetFocusElement() == document_->GetElementById("nav-players") ||
      !focus_tree::is_descendant_of(context_->GetFocusElement(),
                        document_->QuerySelector("main")))
    return false;
  FocusActiveNavigation();

  Rml::Element *players_main = document_->QuerySelector("main");
  Rml::ElementList player_controls;
  players_main->QuerySelectorAll(player_controls, "[data-focus]");
  Rml::Element *last_visible_control = nullptr;
  for (Rml::Element *control : player_controls) {
    if (focus_tree::is_visible_focus(control))
      last_visible_control = control;
  }
  if (!last_visible_control)
    return false;
  last_visible_control->Focus(true);
  for (int index = 0; index < 20; ++index)
    NavigateFocus(0, 1);
  if (!focus_tree::is_descendant_of(context_->GetFocusElement(), players_main) ||
      context_->GetFocusElement()->IsClassSet("quit"))
    return false;
  for (int index = 0; index < 20; ++index)
    NavigateFocus(-1, 0);
  if (!focus_tree::is_descendant_of(context_->GetFocusElement(), players_main))
    return false;
  const Rml::String remembered_player_action =
      context_->GetFocusElement()->GetAttribute<Rml::String>("data-action", "");
  FocusActiveNavigation();
  if (context_->GetFocusElement() != document_->GetElementById("nav-players"))
    return false;
  if (!HandleSdlEvent(activate) || !context_->GetFocusElement() ||
      context_->GetFocusElement()->GetAttribute<Rml::String>(
          "data-action", "") != remembered_player_action)
    return false;

  // Select controls preview before committing a value.
  SelectToolScreen(0);
  Update();
  context_->Update();
  auto *transactional_activity =
      dynamic_cast<Rml::ElementFormControlSelect *>(document_->QuerySelector(
          "[data-action='play-value-activity']"));
  if (!transactional_activity)
    return false;
  const std::string activity_before_preview = state_.activity;
  transactional_activity->Focus(true);
  if (!HandleSdlEvent(activate) ||
      !transactional_activity->IsSelectBoxVisible())
    return false;
  HandleSdlEvent(choose_next);
  if (state_.activity != activity_before_preview ||
      !transactional_activity->IsSelectBoxVisible())
    return false;
  HandleSdlEvent(activate);
  if (state_.activity == activity_before_preview)
    return false;
  Update();
  context_->Update();

  // Detail panes and tabs expose deliberate return paths.
  SelectToolScreen(1);
  Update();
  context_->Update();
  Rml::Element *first_quest =
      document_->QuerySelector("[data-action='quest-violet']");
  if (!first_quest)
    return false;
  first_quest->Focus(true);
  NavigateFocus(1, 0);
  if (!context_->GetFocusElement() ||
      !context_->GetFocusElement()->IsClassSet("detail"))
    return false;
  ActivateFocus();
  if (!context_->GetFocusElement() ||
      context_->GetFocusElement()->GetAttribute<Rml::String>(
          "data-action", "") != "play-lobby")
    return false;
  first_quest->Focus(true);
  NavigateFocus(0, -1);
  if (!context_->GetFocusElement() ||
      context_->GetFocusElement()->GetAttribute<Rml::String>(
          "data-action", "") != "play-lobby")
    return false;
  first_quest->Focus(true);
  if (!HandleSdlEvent(controller_back) || state_.play_view != PlayView::Lobby)
    return false;

  SelectToolScreen(5);
  Update();
  context_->Update();
  constexpr const char *profile_actions[]{"profile-moss", "profile-vega",
                                          "profile-guest"};
  for (const char *profile_action : profile_actions) {
    Rml::Element *profile = document_->QuerySelector(
        (std::string("[data-action='") + profile_action + "']").c_str());
    if (!profile)
      return false;
    profile->Focus(true);
    NavigateFocus(0, -1);
    if (!context_->GetFocusElement() ||
        context_->GetFocusElement()->GetAttribute<Rml::String>(
            "data-action", "") != "new-profile")
      return false;
  }
  NavigateFocus(0, -1);
  if (!context_->GetFocusElement() ||
      context_->GetFocusElement()->GetAttribute<Rml::String>(
          "data-action", "") != "player-tab-Profiles")
    return false;

  SelectToolScreen(4);
  Update();
  context_->Update();
  Rml::Element *assign_device =
      document_->QuerySelector("[data-action='assign-device']");
  if (!assign_device)
    return false;
  assign_device->Focus(true);
  NavigateFocus(0, 1);
  if (context_->GetFocusElement() != assign_device)
    return false;
  NavigateFocus(1, 0);
  if (!context_->GetFocusElement() ||
      context_->GetFocusElement()->GetAttribute<Rml::String>(
          "data-action", "") != "toggle-ready")
    return false;

  SelectToolScreen(7);
  Update();
  context_->Update();
  Rml::Element *display_tab =
      document_->QuerySelector("[data-action='settings-tab-Display']");
  if (!display_tab)
    return false;
  display_tab->Focus(true);
  NavigateFocus(1, 0);
  if (state_.settings_tab != "Audio" ||
      !context_->GetFocusElement() ||
      context_->GetFocusElement()->GetAttribute<Rml::String>(
          "data-action", "") != "settings-tab-Audio")
    return false;
  Update();
  context_->Update();

  // Analog navigation and native select boxes remain reachable.
  SelectToolScreen(0);
  Update();
  context_->Update();
  nav_play = document_->GetElementById("nav-play");
  nav_play->Focus(true);
  SDL_Event stick_navigation{};
  stick_navigation.type = SDL_EVENT_GAMEPAD_AXIS_MOTION;
  stick_navigation.gaxis.axis = SDL_GAMEPAD_AXIS_LEFTX;
  stick_navigation.gaxis.value = 24000;
  HandleSdlEvent(stick_navigation);
  if (context_->GetFocusElement() == nav_play)
    return false;
  stick_navigation.gaxis.value = 0;
  HandleSdlEvent(stick_navigation);
  state_.mod_filter.clear();
  state_.selected_mod = "Mycelium Below";
  toast_.clear();
  if (Rml::Element *toast = document_->GetElementById("toast")) {
    toast->SetInnerRML("");
    toast->SetClass("visible", false);
  }
  SelectToolScreen(13);
  Update();
  context_->Update();
  auto *open_select = dynamic_cast<Rml::ElementFormControlSelect *>(
      document_->QuerySelector("[data-action='tuning-value-response-curve']"));
  if (!open_select)
    return false;
  open_select->Focus(true);
  open_select->ShowSelectBox();
  if (!open_select->IsSelectBoxVisible())
    return false;
  return true;
}
