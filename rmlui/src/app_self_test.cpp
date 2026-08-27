#include "app.h"
#include "focus_elements.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Elements/ElementFormControl.h>
#include <RmlUi/Core/Elements/ElementFormControlInput.h>
#include <RmlUi/Core/Elements/ElementFormControlSelect.h>
#include <RmlUi/Core/Event.h>
#include <cmath>

bool GubsyApp::RunSelfTest() {
  // drive authored actions through the retained document
  auto click = [&](const char *action) {
    Rml::ElementList candidates;
    document_->QuerySelectorAll(candidates, "[data-action]");
    for (Rml::Element *candidate : candidates) {
      if (candidate->GetAttribute<Rml::String>("data-action", "") == action) {
        candidate->Click();
        Update();
        context_->Update();
        return true;
      }
    }
    return false;
  };

  // test routes, providers, and guarded mutations
  if (!click("nav-controls") || state_.destination != Destination::Controls)
    return false;
  if (!click("controls-tab-Devices") || state_.controls_tab != "Devices")
    return false;
  if (!click("device-xbox") ||
      state_.selected_device != "Xbox Wireless Controller")
    return false;
  if (!click("nav-play"))
    return false;
  auto *activity = dynamic_cast<Rml::ElementFormControl *>(
      document_->QuerySelector("[data-action='play-value-activity']"));
  if (!activity)
    return false;
  activity->SetValue("New expedition");
  activity->DispatchEvent(Rml::EventId::Change, {});
  Update();
  context_->Update();
  if (state_.activity != "New expedition" ||
      document_->GetInnerRML().find("Starting quest") == std::string::npos)
    return false;
  if (!click("play-rules") || state_.play_view != PlayView::Rules)
    return false;
  auto *rule_scroll = document_->QuerySelector(".rules .scroll-body");
  auto *shortcuts = dynamic_cast<Rml::ElementFormControlInput *>(
      document_->QuerySelector("[data-action='rule-value-shortcuts']"));
  if (!rule_scroll || !shortcuts)
    return false;
  rule_scroll->SetScrollTop(80.0f);
  const float scroll_before_toggle = rule_scroll->GetScrollTop();
  shortcuts->RemoveAttribute("checked");
  if (state_.shortcuts ||
      std::abs(rule_scroll->GetScrollTop() - scroll_before_toggle) > 0.5f)
    return false;
  SetProviderState(3);
  Update();
  context_->Update();
  if (!click("retry-provider") || state_.provider_state != "Populated")
    return false;
  SelectToolScreen(15);
  Update();
  context_->Update();
  if (!click("uninstall-plan") || state_.modal.empty())
    return false;
  if (!context_->GetFocusElement() ||
      context_->GetFocusElement()->GetAttribute<Rml::String>(
          "data-action", "") != "cancel-modal")
    return false;
  NavigateFocus(1, 0);
  if (!context_->GetFocusElement() ||
      context_->GetFocusElement()->GetAttribute<Rml::String>(
          "data-action", "") != "confirm-modal")
    return false;
  if (!click("cancel-modal") || !state_.modal.empty())
    return false;

  // test binding capture and local tabs
  SelectToolScreen(11);
  Update();
  context_->Update();
  SDL_Event shoulder{};
  shoulder.type = SDL_EVENT_GAMEPAD_BUTTON_DOWN;
  shoulder.gbutton.button = SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER;
  if (!HandleSdlEvent(shoulder) || state_.controls_tab != "Devices")
    return false;
  shoulder.gbutton.button = SDL_GAMEPAD_BUTTON_LEFT_SHOULDER;
  if (!HandleSdlEvent(shoulder) || state_.controls_tab != "Bindings")
    return false;
  Update();
  context_->Update();
  if (!click("capture-binding") || !state_.capture_mode)
    return false;
  SDL_Event synthetic{};
  synthetic.type = SDL_EVENT_GAMEPAD_BUTTON_DOWN;
  synthetic.gbutton.button = SDL_GAMEPAD_BUTTON_NORTH;
  if (!HandleSdlEvent(synthetic) || state_.capture_mode)
    return false;

  // test session and catalog package state
  SelectToolScreen(3);
  Update();
  context_->Update();
  if (!click("session-browse") || !state_.session_mod_browse)
    return false;
  if (!click("session-mod-Skybreak Caverns") ||
      state_.selected_mod != "Skybreak Caverns")
    return false;
  if (!click("install-add-session") || state_.session_mod_browse)
    return false;

  SelectToolScreen(16);
  Update();
  context_->Update();
  if (auto *filter = dynamic_cast<Rml::ElementFormControl *>(
          document_->QuerySelector("[data-action='search-catalog']"))) {
    filter->SetValue("Mirror");
    filter->DispatchEvent(Rml::EventId::Change, {});
    Update();
    context_->Update();
    if (!document_->QuerySelector(
            "[data-action='catalog-mod-Mirror Depths']") ||
        document_->QuerySelector("[data-action='catalog-mod-Mycelium Below']"))
      return false;
  } else {
    return false;
  }

  // test native setting scroll and edit state
  SelectToolScreen(7);
  Update();
  context_->Update();
  auto *resolution = dynamic_cast<Rml::ElementFormControl *>(
      document_->QuerySelector("[data-action='setting-value-resolution']"));
  auto *brightness = dynamic_cast<Rml::ElementFormControl *>(
      document_->QuerySelector("[data-action='setting-value-brightness']"));
  if (!resolution || !brightness)
    return false;
  resolution->SetValue("1280 × 720");
  resolution->DispatchEvent(Rml::EventId::Change, {});
  brightness->SetValue("77");
  brightness->DispatchEvent(Rml::EventId::Change, {});
  if (state_.setting_values["resolution"] != "1280 × 720" ||
      state_.setting_values["brightness"] != "77" ||
      state_.selected_setting != "Brightness")
    return false;
  auto *fullscreen = dynamic_cast<Rml::ElementFormControl *>(
      document_->QuerySelector("[data-action='setting-value-fullscreen']"));
  Rml::Element *settings_scroll =
      document_->QuerySelector(".settings-workspace .scroll-body");
  if (!fullscreen || !settings_scroll)
    return false;
  settings_scroll->SetScrollTop(60.0f);
  const float settings_scroll_before = settings_scroll->GetScrollTop();
  const Rml::Vector2f checkbox_position =
      fullscreen->GetAbsoluteOffset(Rml::BoxArea::Border) +
      fullscreen->GetBox().GetSize(Rml::BoxArea::Border) * 0.5f;
  context_->ProcessMouseMove(static_cast<int>(checkbox_position.x),
                             static_cast<int>(checkbox_position.y), 0);
  context_->ProcessMouseButtonDown(0, 0);
  context_->ProcessMouseButtonUp(0, 0);
  if (state_.setting_values["fullscreen"] != "false" ||
      std::abs(settings_scroll->GetScrollTop() - settings_scroll_before) > 0.5f)
    return false;
  SDL_Event activate{};
  activate.type = SDL_EVENT_GAMEPAD_BUTTON_DOWN;
  activate.gbutton.button = SDL_GAMEPAD_BUTTON_SOUTH;
  brightness->Focus(true);
  if (!HandleSdlEvent(activate) || !controller_editing_)
    return false;
  SDL_Event adjust_range{};
  adjust_range.type = SDL_EVENT_GAMEPAD_BUTTON_DOWN;
  adjust_range.gbutton.button = SDL_GAMEPAD_BUTTON_DPAD_RIGHT;
  if (!HandleSdlEvent(adjust_range) ||
      state_.setting_values["brightness"] != "78")
    return false;
  if (!HandleSdlEvent(activate) || controller_editing_)
    return false;
  resolution->Focus(true);
  if (!HandleSdlEvent(adjust_range) ||
      state_.setting_values["resolution"] != "1280 × 720")
    return false;

  // test controller tuning transactions
  SelectToolScreen(13);
  Update();
  context_->Update();
  auto *look =
      dynamic_cast<Rml::ElementFormControlInput *>(document_->QuerySelector(
          "[data-action='tuning-value-look-sensitivity']"));
  auto *curve = dynamic_cast<Rml::ElementFormControlSelect *>(
      document_->QuerySelector("[data-action='tuning-value-response-curve']"));
  auto *invert = dynamic_cast<Rml::ElementFormControlInput *>(
      document_->QuerySelector("[data-action='tuning-value-invert-y']"));
  if (!look || !curve || !invert)
    return false;
  look->Focus(true);
  if (!HandleSdlEvent(activate) || !controller_editing_)
    return false;
  if (!HandleSdlEvent(adjust_range) ||
      state_.tuning_values["look-sensitivity"] != "46")
    return false;
  if (!HandleSdlEvent(activate) || controller_editing_)
    return false;
  curve->Focus(true);
  if (!HandleSdlEvent(activate) || !curve->IsSelectBoxVisible())
    return false;
  SDL_Event choose_next{};
  choose_next.type = SDL_EVENT_GAMEPAD_BUTTON_DOWN;
  choose_next.gbutton.button = SDL_GAMEPAD_BUTTON_DPAD_DOWN;
  HandleSdlEvent(choose_next);
  HandleSdlEvent(activate);
  if (curve->IsSelectBoxVisible() ||
      state_.tuning_values["response-curve"] == "Smooth")
    return false;
  invert->Focus(true);
  if (!HandleSdlEvent(activate) || state_.tuning_values["invert-y"] != "true")
    return false;

  return RunNavigationSelfTest();
}
