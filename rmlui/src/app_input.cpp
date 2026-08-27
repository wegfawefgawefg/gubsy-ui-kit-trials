#include "app.h"
#include "focus_elements.h"
#include "ui_markup.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Elements/ElementFormControl.h>
#include <RmlUi/Core/Elements/ElementFormControlInput.h>
#include <RmlUi/Core/Elements/ElementFormControlSelect.h>
#include <RmlUi/Core/Event.h>

#include <algorithm>
#include <cmath>
#include <unordered_map>

// Native control changes and SDL keyboard/gamepad dispatch.

void GubsyApp::ProcessEvent(Rml::Event &event) {
  if (event.GetId() != Rml::EventId::Click &&
      event.GetId() != Rml::EventId::Change)
    return;
  Rml::Element *element = focus_tree::action_element(event.GetTargetElement());
  if (!element)
    return;
  const std::string action =
      element->GetAttribute<Rml::String>("data-action", "");
  if (event.GetId() == Rml::EventId::Change) {
    if (auto *control = dynamic_cast<Rml::ElementFormControl *>(element)) {
      if (controller_editing_ && element == context_->GetFocusElement() &&
          dynamic_cast<Rml::ElementFormControlSelect *>(element))
        return;
      if (action == "filter-actions")
        state_.control_filter = control->GetValue();
      else if (action == "search-catalog")
        state_.mod_filter = control->GetValue();
      else if (action.rfind("play-value-", 0) == 0) {
        const std::string id = action.substr(11);
        if (id == "activity")
          state_.activity = control->GetValue();
        else if (id == "access")
          state_.access = control->GetValue();
        else if (id == "host")
          state_.host = control->GetValue();
        MarkDirty();
        return;
      } else if (action.rfind("rule-value-", 0) == 0) {
        const std::string id = action.substr(11);
        std::string value = control->GetValue();
        if (element->GetAttribute<Rml::String>("type", "") == "checkbox")
          value = event.GetParameter<bool>("checked", false) ? "true" : "false";
        else if (element->GetAttribute<Rml::String>("type", "") == "range")
          value =
              std::to_string(static_cast<int>(std::lround(std::stof(value))));
        static const std::unordered_map<std::string, std::string> names{
            {"difficulty", "Expedition difficulty"},
            {"variation", "Stage variation"},
            {"shared-lives", "Shared lives"},
            {"starting-health", "Starting health"},
            {"ghost-arrival", "Ghost arrival"},
            {"shop-frequency", "Shop frequency"},
            {"shortcuts", "Discovered shortcuts"},
            {"treasury", "Shared treasury"},
            {"friendly-fire", "Friendly fire"},
            {"lantern-fuel", "Lantern fuel"}};
        if (id == "shortcuts")
          state_.shortcuts = value == "true";
        else if (id == "treasury")
          state_.shared_treasury = value == "true";
        else if (id == "friendly-fire")
          state_.friendly_fire = value == "true";
        else
          state_.rule_values[id] = value;
        state_.selected_rule = names.at(id);
        std::string formatted = value;
        if (value == "true" || value == "false")
          formatted = value == "true" ? "ON" : "OFF";
        else if (id == "starting-health")
          formatted += " hearts";
        else if (id == "ghost-arrival")
          formatted += " sec";
        if (Rml::Element *label =
                document_->GetElementById("rule-output-" + id))
          label->SetInnerRML(formatted);
        if (Rml::Element *title =
                document_->GetElementById("rule-detail-title"))
          title->SetInnerRML(state_.selected_rule);
        if (Rml::Element *current =
                document_->GetElementById("rule-detail-current"))
          current->SetInnerRML(formatted);
        return;
      } else if (action.rfind("tuning-value-", 0) == 0) {
        const std::string id = action.substr(13);
        std::string value = control->GetValue();
        if (element->GetAttribute<Rml::String>("type", "") == "checkbox")
          value = event.GetParameter<bool>("checked", false) ? "true" : "false";
        else if (element->GetAttribute<Rml::String>("type", "") == "range")
          value =
              std::to_string(static_cast<int>(std::lround(std::stof(value))));
        state_.tuning_values[id] = value;
        const std::string formatted = value == "true"    ? "ON"
                                      : value == "false" ? "OFF"
                                                         : value + "%";
        if (Rml::Element *label =
                document_->GetElementById("tuning-output-" + id))
          label->SetInnerRML(formatted);
        if (Rml::Element *summary =
                document_->GetElementById("tuning-summary-" + id))
          summary->SetInnerRML(id == "response-curve" ? value : formatted);
        return;
      } else if (action.rfind("choice-value-", 0) == 0) {
        state_.choice_values[action.substr(13)] = control->GetValue();
        return;
      } else if (action == "compatible-value") {
        state_.compatible_only = event.GetParameter<bool>("checked", false);
        Rml::ElementList incompatible;
        document_->QuerySelectorAll(incompatible, ".catalog-row.incompatible");
        for (Rml::Element *row : incompatible) {
          if (state_.compatible_only)
            row->SetProperty("display", "none");
          else
            row->RemoveProperty("display");
        }
        return;
      } else if (action.rfind("setting-value-", 0) == 0) {
        const std::string id = action.substr(14);
        std::string value = control->GetValue();
        if (element->GetAttribute<Rml::String>("type", "") == "checkbox")
          value = event.GetParameter<bool>("checked", false) ? "true" : "false";
        if (id == "render-scale" || id == "brightness" ||
            id == "master-volume" || id == "music-volume" ||
            id == "effects-volume" || id == "dialogue-volume" ||
            id == "camera-shake")
          value =
              std::to_string(static_cast<int>(std::lround(std::stof(value))));
        state_.setting_values[id] = value.empty() ? "false" : value;
        state_.selected_setting = element->GetAttribute<Rml::String>(
            "data-setting-name", state_.selected_setting);
        Rml::ElementList rows;
        document_->QuerySelectorAll(rows, ".setting-row");
        for (Rml::Element *row : rows)
          row->SetClass("selected", false);
        if (Rml::Element *row = document_->GetElementById("setting-row-" + id))
          row->SetClass("selected", true);
        const std::string formatted =
            markup::format_setting_value(id, state_.setting_values[id]);
        if (Rml::Element *label = document_->GetElementById("value-" + id))
          label->SetInnerRML(formatted);
        if (Rml::Element *title =
                document_->GetElementById("setting-detail-title"))
          title->SetInnerRML(state_.selected_setting);
        if (Rml::Element *current =
                document_->GetElementById("setting-detail-current"))
          current->SetInnerRML(formatted);
        return;
      } else {
        HandleAction(action);
        return;
      }
      MarkDirty();
      return;
    }
  }
  if (action.rfind("setting-value-", 0) == 0)
    return;
  HandleAction(action);
}

bool GubsyApp::HandleSdlEvent(const SDL_Event &event) {
  if (state_.capture_mode && event.type == SDL_EVENT_KEY_DOWN &&
      !event.key.repeat) {
    state_.capture_mode = false;
    SetToast("Captured keyboard scancode " +
             std::to_string(event.key.scancode));
    MarkDirty();
    return true;
  }
  if (state_.capture_mode && event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
    state_.capture_mode = false;
    SetToast("Captured gamepad button " + std::to_string(event.gbutton.button));
    MarkDirty();
    return true;
  }
  if (event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
    state_.raw_input_name =
        "Axis " + std::to_string(event.gaxis.axis) + " · live gamepad input";
    state_.raw_input_value = std::clamp(
        std::abs(static_cast<float>(event.gaxis.value)) / 32767.0f, 0.0f, 1.0f);
    if (state_.destination == Destination::Controls &&
        state_.controls_tab == "Devices")
      MarkDirty();
    constexpr int axis_threshold = 18000;
    if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX) {
      const int direction = event.gaxis.value > axis_threshold    ? 1
                            : event.gaxis.value < -axis_threshold ? -1
                                                                  : 0;
      if (direction != 0 && direction != controller_x_latch_)
        NavigateFocus(direction, 0);
      controller_x_latch_ = direction;
      return direction != 0;
    }
    if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTY) {
      const int direction = event.gaxis.value > axis_threshold    ? 1
                            : event.gaxis.value < -axis_threshold ? -1
                                                                  : 0;
      if (direction != 0 && direction != controller_y_latch_)
        NavigateFocus(0, direction);
      controller_y_latch_ = direction;
      return direction != 0;
    }
  }
  if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
    if (dynamic_cast<Rml::ElementFormControl *>(context_->GetFocusElement()) &&
        event.key.key != SDLK_ESCAPE)
      return false;
    switch (event.key.key) {
    case SDLK_LEFT:
      NavigateFocus(-1, 0);
      return true;
    case SDLK_RIGHT:
      NavigateFocus(1, 0);
      return true;
    case SDLK_UP:
      NavigateFocus(0, -1);
      return true;
    case SDLK_DOWN:
      NavigateFocus(0, 1);
      return true;
    case SDLK_RETURN:
    case SDLK_SPACE:
      ActivateFocus();
      return true;
    case SDLK_ESCAPE:
      Back();
      return true;
    default:
      break;
    }
  }
  if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
    if (auto *select = dynamic_cast<Rml::ElementFormControlSelect *>(
            context_->GetFocusElement());
        select && select->IsSelectBoxVisible()) {
      Rml::Input::KeyIdentifier key = Rml::Input::KI_UNKNOWN;
      if (event.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_UP)
        key = Rml::Input::KI_UP;
      else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_DOWN)
        key = Rml::Input::KI_DOWN;
      else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH)
        key = Rml::Input::KI_RETURN;
      else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_EAST)
        key = Rml::Input::KI_ESCAPE;
      if (key != Rml::Input::KI_UNKNOWN) {
        context_->ProcessKeyDown(key, 0);
        context_->ProcessKeyUp(key, 0);
        if (event.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH) {
          select->HideSelectBox();
          controller_editing_ = false;
          select->SetClass("controller-editing", false);
          select->DispatchEvent(Rml::EventId::Change, {});
        } else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_EAST) {
          select->CancelSelectBox();
          controller_editing_ = false;
          select->SetClass("controller-editing", false);
        }
        return true;
      }
    }
    if (controller_editing_ && context_->GetFocusElement()) {
      auto *input = dynamic_cast<Rml::ElementFormControlInput *>(
          context_->GetFocusElement());
      if (!input || input->GetAttribute<Rml::String>("type", "") != "range") {
        controller_editing_ = false;
      } else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_LEFT ||
                 event.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_RIGHT) {
      const int direction =
          event.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_RIGHT ? 1 : -1;
        const float minimum = input->GetAttribute<float>("min", 0.0f);
        const float maximum = input->GetAttribute<float>("max", 100.0f);
        const float step = input->GetAttribute<float>("step", 1.0f);
        const float value = std::clamp(std::stof(input->GetValue()) +
                                           step * static_cast<float>(direction),
                                       minimum, maximum);
        input->SetValue(std::to_string(value));
        input->DispatchEvent(Rml::EventId::Change, {});
        return true;
      } else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH) {
        controller_editing_ = false;
        input->SetClass("controller-editing", false);
        SetToast("Value accepted");
        return true;
      } else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_EAST) {
        input->SetValue(controller_edit_original_value_);
        input->DispatchEvent(Rml::EventId::Change, {});
        controller_editing_ = false;
        input->SetClass("controller-editing", false);
        SetToast("Edit cancelled");
        return true;
      }
      return true;
    }
    switch (event.gbutton.button) {
    case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
      NavigateFocus(-1, 0);
      return true;
    case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
      NavigateFocus(1, 0);
      return true;
    case SDL_GAMEPAD_BUTTON_DPAD_UP:
      NavigateFocus(0, -1);
      return true;
    case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
      NavigateFocus(0, 1);
      return true;
    case SDL_GAMEPAD_BUTTON_SOUTH:
      ActivateFocus();
      return true;
    case SDL_GAMEPAD_BUTTON_EAST:
      if (state_.destination == Destination::Play &&
          state_.play_view != PlayView::Lobby)
        Back();
      else if (Rml::Element *main = document_->QuerySelector("main");
               main && focus_tree::is_descendant_of(context_->GetFocusElement(), main))
        FocusActiveNavigation();
      else
        Back();
      return true;
    case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
      HandleAction("local-tab-prev");
      return true;
    case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
      HandleAction("local-tab-next");
      return true;
    default:
      break;
    }
  }
  return false;
}
