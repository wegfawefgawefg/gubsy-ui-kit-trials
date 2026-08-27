#include "app.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Elements/ElementFormControl.h>
#include <RmlUi/Core/Elements/ElementFormControlInput.h>
#include <RmlUi/Core/Elements/ElementFormControlSelect.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/SystemInterface.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <utility>

namespace {

std::string selected_class(bool selected, const char *base = "") {
  std::string result(base);
  if (selected)
    result += (result.empty() ? "selected" : " selected");
  return result;
}

std::string selected_class(bool selected, const std::string &base) {
  return selected_class(selected, base.c_str());
}

std::string on_off(bool value) { return value ? "ON" : "OFF"; }

bool contains_ci(const std::string &text, const std::string &query) {
  if (query.empty())
    return true;
  auto fold = [](unsigned char character) { return std::tolower(character); };
  return std::search(text.begin(), text.end(), query.begin(), query.end(),
                     [&](char left, char right) {
                       return fold(static_cast<unsigned char>(left)) ==
                              fold(static_cast<unsigned char>(right));
                     }) != text.end();
}

std::string escape_attribute(const std::string &value) {
  std::string result;
  result.reserve(value.size());
  for (const char character : value) {
    switch (character) {
    case '&':
      result += "&amp;";
      break;
    case '"':
      result += "&quot;";
      break;
    case '<':
      result += "&lt;";
      break;
    case '>':
      result += "&gt;";
      break;
    default:
      result += character;
      break;
    }
  }
  return result;
}

std::string format_setting_value(const std::string &id,
                                 const std::string &value) {
  if (value == "true")
    return "ON";
  if (value == "false" || value.empty())
    return "OFF";
  if (id == "brightness" || id.find("volume") != std::string::npos ||
      id == "camera-shake" || id == "render-scale")
    return value + "%";
  return value;
}

void append_select(std::ostringstream &out, const std::string &action,
                   const std::string &value,
                   std::initializer_list<const char *> options,
                   const char *class_name = "native-select") {
  out << R"(<select data-focus data-action=")" << action << R"(" class=")"
      << class_name << R"(">)";
  for (const char *option : options) {
    out << R"(<option value=")" << escape_attribute(option) << R"(")";
    if (value == option)
      out << R"( selected="selected")";
    out << R"(>)" << option << R"(</option>)";
  }
  out << R"(</select>)";
}

std::string
map_value(const std::unordered_map<std::string, std::string> &values,
          const std::string &id) {
  const auto it = values.find(id);
  return it == values.end() ? std::string{} : it->second;
}

std::string mod_image(size_t index) {
  return "sheet-" + std::to_string(index / 5 + 1) + "-" +
         std::to_string(index % 5) + ".png";
}

Rml::Element *action_element(Rml::Element *element) {
  while (element) {
    if (element->HasAttribute("data-action"))
      return element;
    element = element->GetParentNode();
  }
  return nullptr;
}

bool is_descendant_of(Rml::Element *element, Rml::Element *ancestor) {
  for (; element; element = element->GetParentNode()) {
    if (element == ancestor)
      return true;
  }
  return false;
}

Rml::Element *ancestor_with_class(Rml::Element *element,
                                  const char *class_name) {
  for (; element; element = element->GetParentNode()) {
    if (element->IsClassSet(class_name))
      return element;
  }
  return nullptr;
}

bool is_visible_focus(Rml::Element *element) {
  return element && element->GetOffsetWidth() > 0 &&
         element->GetOffsetHeight() > 0;
}

} // namespace

GubsyApp::GubsyApp(Rml::Context *context) : context_(context) {}

GubsyApp::~GubsyApp() {
  if (!document_)
    return;
  document_->RemoveEventListener(Rml::EventId::Click, this);
  document_->RemoveEventListener(Rml::EventId::Change, this);
  document_->Close();
}

bool GubsyApp::Initialize(const std::string &document_path) {
  if (!context_)
    return false;
  document_ = context_->LoadDocument(document_path);
  if (!document_)
    return false;
  document_->AddEventListener(Rml::EventId::Click, this);
  document_->AddEventListener(Rml::EventId::Change, this);
  document_->Show();
  Render();
  return true;
}

void GubsyApp::MarkDirty() { dirty_ = true; }

void GubsyApp::Update() {
  if (dirty_)
    Render();
  if (!toast_.empty() &&
      Rml::GetSystemInterface()->GetElapsedTime() > toast_until_) {
    toast_.clear();
    if (Rml::Element *toast = document_->GetElementById("toast"))
      toast->SetClass("visible", false);
  }
}

void GubsyApp::SetViewport(int width, int height) {
  viewport_width_ = width;
  viewport_height_ = height;
  if (document_)
    RenderChrome();
}

const char *GubsyApp::current_screen_name() const {
  if (state_.destination == Destination::Play) {
    switch (state_.play_view) {
    case PlayView::Lobby:
      return "Play lobby";
    case PlayView::Quest:
      return "Quest picker";
    case PlayView::Rules:
      return "Session settings";
    case PlayView::SessionMods:
      return "Session mods";
    }
  }
  switch (state_.destination) {
  case Destination::Players:
    return "Players";
  case Destination::Settings:
    return "Settings";
  case Destination::Controls:
    return "Controls";
  case Destination::Progress:
    return "Progress";
  case Destination::Mods:
    return "Mods";
  case Destination::Play:
    break;
  }
  return "Play";
}

void GubsyApp::SelectToolScreen(int index) {
  switch (index) {
  case 0:
    state_.destination = Destination::Play;
    state_.play_view = PlayView::Lobby;
    break;
  case 1:
    state_.destination = Destination::Play;
    state_.play_view = PlayView::Quest;
    break;
  case 2:
    state_.destination = Destination::Play;
    state_.play_view = PlayView::Rules;
    break;
  case 3:
    state_.destination = Destination::Play;
    state_.play_view = PlayView::SessionMods;
    break;
  case 4:
    state_.destination = Destination::Players;
    break;
  case 5:
    state_.destination = Destination::Players;
    state_.player_tab = "Profiles";
    break;
  case 6:
    state_.destination = Destination::Players;
    state_.player_tab = "Devices";
    break;
  case 7:
    state_.destination = Destination::Settings;
    state_.settings_tab = "Display";
    state_.selected_setting = "Fullscreen";
    break;
  case 8:
    state_.destination = Destination::Settings;
    state_.settings_tab = "Audio";
    state_.selected_setting = "Master volume";
    break;
  case 9:
    state_.destination = Destination::Settings;
    state_.settings_tab = "Accessibility";
    state_.selected_setting = "Text scale";
    break;
  case 10:
    state_.destination = Destination::Settings;
    state_.settings_tab = "Gameplay";
    state_.selected_setting = "Context tutorials";
    break;
  case 11:
    state_.destination = Destination::Controls;
    state_.controls_tab = "Bindings";
    break;
  case 12:
    state_.destination = Destination::Controls;
    state_.controls_tab = "Devices";
    break;
  case 13:
    state_.destination = Destination::Controls;
    state_.controls_tab = "Input tuning";
    break;
  case 14:
    state_.destination = Destination::Progress;
    break;
  case 15:
    state_.destination = Destination::Mods;
    state_.mods_tab = "Installed";
    state_.selected_mod = "Old Lanterns";
    break;
  case 16:
    state_.destination = Destination::Mods;
    state_.mods_tab = "Browse catalog";
    state_.selected_mod = "Mycelium Below";
    break;
  default:
    return;
  }
  MarkDirty();
}

void GubsyApp::SetProviderState(int index) {
  constexpr const char *states[]{"Populated", "Empty", "Loading", "Error"};
  if (index < 0 || index > 3)
    return;
  state_.provider_state = states[index];
  MarkDirty();
}

void GubsyApp::SetGamepadStatus(int count, const std::string &name) {
  if (!document_)
    return;
  if (Rml::Element *status = document_->GetElementById("controller-status")) {
    if (count > 0)
      status->SetInnerRML("● CONTROLLER · " + escape_attribute(name));
    else
      status->SetInnerRML("○ CONNECT CONTROLLER");
  }
}

bool GubsyApp::RunSelfTest() {
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
  brightness->Focus(true);
  SDL_Event adjust_range{};
  adjust_range.type = SDL_EVENT_GAMEPAD_BUTTON_DOWN;
  adjust_range.gbutton.button = SDL_GAMEPAD_BUTTON_DPAD_RIGHT;
  if (!HandleSdlEvent(adjust_range) ||
      state_.setting_values["brightness"] != "78")
    return false;
  resolution->Focus(true);
  if (!HandleSdlEvent(adjust_range) ||
      state_.setting_values["resolution"] != "1920 × 1080")
    return false;

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
  if (!HandleSdlEvent(adjust_range) ||
      state_.tuning_values["look-sensitivity"] != "46")
    return false;
  curve->Focus(true);
  SDL_Event activate{};
  activate.type = SDL_EVENT_GAMEPAD_BUTTON_DOWN;
  activate.gbutton.button = SDL_GAMEPAD_BUTTON_SOUTH;
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

  Rml::Element *players_main = document_->QuerySelector("main");
  Rml::ElementList player_controls;
  players_main->QuerySelectorAll(player_controls, "[data-focus]");
  Rml::Element *last_visible_control = nullptr;
  for (Rml::Element *control : player_controls) {
    if (is_visible_focus(control))
      last_visible_control = control;
  }
  if (!last_visible_control)
    return false;
  last_visible_control->Focus(true);
  for (int index = 0; index < 20; ++index)
    NavigateFocus(0, 1);
  if (!is_descendant_of(context_->GetFocusElement(), players_main) ||
      context_->GetFocusElement()->IsClassSet("quit"))
    return false;
  for (int index = 0; index < 20; ++index)
    NavigateFocus(-1, 0);
  if (context_->GetFocusElement() != document_->GetElementById("nav-players"))
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

void GubsyApp::ProcessEvent(Rml::Event &event) {
  if (event.GetId() != Rml::EventId::Click &&
      event.GetId() != Rml::EventId::Change)
    return;
  Rml::Element *element = action_element(event.GetTargetElement());
  if (!element)
    return;
  const std::string action =
      element->GetAttribute<Rml::String>("data-action", "");
  if (event.GetId() == Rml::EventId::Change) {
    if (auto *control = dynamic_cast<Rml::ElementFormControl *>(element)) {
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
            format_setting_value(id, state_.setting_values[id]);
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
        if (event.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH)
          select->HideSelectBox();
        else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_EAST)
          select->CancelSelectBox();
        return true;
      }
    }
    if ((event.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_LEFT ||
         event.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_RIGHT) &&
        context_->GetFocusElement()) {
      const int direction =
          event.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_RIGHT ? 1 : -1;
      if (auto *input = dynamic_cast<Rml::ElementFormControlInput *>(
              context_->GetFocusElement());
          input && input->GetAttribute<Rml::String>("type", "") == "range") {
        const float minimum = input->GetAttribute<float>("min", 0.0f);
        const float maximum = input->GetAttribute<float>("max", 100.0f);
        const float step = input->GetAttribute<float>("step", 1.0f);
        const float value = std::clamp(std::stof(input->GetValue()) +
                                           step * static_cast<float>(direction),
                                       minimum, maximum);
        input->SetValue(std::to_string(value));
        input->DispatchEvent(Rml::EventId::Change, {});
        return true;
      }
      if (auto *select = dynamic_cast<Rml::ElementFormControlSelect *>(
              context_->GetFocusElement())) {
        const int count = select->GetNumOptions();
        if (count > 0) {
          const int next = (select->GetSelection() + direction + count) % count;
          select->SetSelection(next);
          select->DispatchEvent(Rml::EventId::Change, {});
        }
        return true;
      }
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

void GubsyApp::NavigateFocus(int dx, int dy) {
  if (!document_)
    return;

  Rml::ElementList candidates;
  document_->QuerySelectorAll(candidates, state_.modal.empty()
                                              ? "[data-focus]"
                                              : "#modal-root [data-focus]");
  candidates.erase(
      std::remove_if(candidates.begin(), candidates.end(),
                     [](Rml::Element *element) {
                       return !is_visible_focus(element);
                     }),
      candidates.end());
  if (candidates.empty())
    return;

  Rml::Element *current = context_->GetFocusElement();
  auto focus = [](Rml::Element *element, bool activate = false) {
    if (!element)
      return;
    element->Focus(true);
    element->ScrollIntoView(false);
    if (activate)
      element->Click();
  };

  auto geometric_target = [&](Rml::Element *origin,
                              const Rml::ElementList &pool) {
    if (!origin)
      return static_cast<Rml::Element *>(nullptr);
    const Rml::Vector2f current_position =
        origin->GetAbsoluteOffset(Rml::BoxArea::Border);
    const float cx = current_position.x + origin->GetOffsetWidth() * 0.5f;
    const float cy = current_position.y + origin->GetOffsetHeight() * 0.5f;
    Rml::Element *best = nullptr;
    float best_score = std::numeric_limits<float>::max();
    for (Rml::Element *candidate : pool) {
      if (candidate == origin || !is_visible_focus(candidate))
        continue;
      const Rml::Vector2f position =
          candidate->GetAbsoluteOffset(Rml::BoxArea::Border);
      const float x = position.x + candidate->GetOffsetWidth() * 0.5f;
      const float y = position.y + candidate->GetOffsetHeight() * 0.5f;
      const float delta_x = x - cx;
      const float delta_y = y - cy;
      const float forward = delta_x * dx + delta_y * dy;
      if (forward <= 1.0f)
        continue;
      const float lateral = std::abs(delta_x * dy - delta_y * dx);
      const float score =
          forward + lateral * 3.0f + lateral * lateral / forward;
      if (score < best_score) {
        best_score = score;
        best = candidate;
      }
    }
    return best;
  };

  if (!state_.modal.empty()) {
    if (!current || std::find(candidates.begin(), candidates.end(), current) ==
                        candidates.end())
      focus(candidates.front());
    else
      focus(geometric_target(current, candidates));
    return;
  }

  const char *active_nav_id = "nav-play";
  switch (state_.destination) {
  case Destination::Players:
    active_nav_id = "nav-players";
    break;
  case Destination::Settings:
    active_nav_id = "nav-settings";
    break;
  case Destination::Controls:
    active_nav_id = "nav-controls";
    break;
  case Destination::Progress:
    active_nav_id = "nav-progress";
    break;
  case Destination::Mods:
    active_nav_id = "nav-mods";
    break;
  case Destination::Play:
    break;
  }
  Rml::Element *active_nav = document_->GetElementById(active_nav_id);
  Rml::Element *main = document_->QuerySelector("main");
  Rml::Element *nav = document_->QuerySelector("nav");
  const bool compact_horizontal_nav =
      viewport_width_ <= 700 ||
      (viewport_height_ <= 500 && viewport_width_ <= 1000);

  const auto is_primary_nav = [](Rml::Element *element) {
    return element &&
           element->GetAttribute<Rml::String>("id", "").rfind("nav-", 0) == 0;
  };
  const auto is_quit = [](Rml::Element *element) {
    return element && element->IsClassSet("quit");
  };
  constexpr const char *nav_ids[]{"nav-play", "nav-players", "nav-settings",
                                  "nav-controls", "nav-progress", "nav-mods"};
  auto primary_index = [&](Rml::Element *element) {
    for (int index = 0; index < 6; ++index) {
      if (element == document_->GetElementById(nav_ids[index]))
        return index;
    }
    return -1;
  };

  if (!current || std::find(candidates.begin(), candidates.end(), current) ==
                      candidates.end()) {
    focus(active_nav);
    return;
  }

  if (is_primary_nav(current)) {
    const int movement = compact_horizontal_nav ? dx : dy;
    const int index = primary_index(current);
    if (movement != 0) {
      const int next = index + (movement > 0 ? 1 : -1);
      if (next >= 0 && next < 6)
        focus(document_->GetElementById(nav_ids[next]), true);
      else if (!compact_horizontal_nav && next == 6) {
        Rml::Element *quit = nav ? nav->QuerySelector("button.quit") : nullptr;
        focus(quit);
      }
      return;
    }
    const bool enter_content = compact_horizontal_nav ? dy < 0 : dx > 0;
    if (enter_content && main) {
      Rml::ElementList content;
      main->QuerySelectorAll(content, "[data-focus]");
      focus(geometric_target(current, content));
    }
    return;
  }

  if (is_quit(current)) {
    if (!compact_horizontal_nav && dy < 0)
      focus(document_->GetElementById("nav-mods"), true);
    else if (!compact_horizontal_nav && dx > 0 && main) {
      Rml::ElementList content;
      main->QuerySelectorAll(content, "[data-focus]");
      focus(geometric_target(current, content));
    }
    return;
  }

  Rml::Element *tab_bar = ancestor_with_class(current, "local-tabs");
  if (!tab_bar)
    tab_bar = ancestor_with_class(current, "mobile-local-tabs");
  if (tab_bar) {
    if (dx != 0) {
      Rml::ElementList tabs;
      tab_bar->QuerySelectorAll(tabs, "[data-focus]");
      const auto it = std::find(tabs.begin(), tabs.end(), current);
      if (it != tabs.end()) {
        const int index = static_cast<int>(it - tabs.begin());
        const int next = index + (dx > 0 ? 1 : -1);
        if (next >= 0 && next < static_cast<int>(tabs.size()))
          focus(tabs[next], true);
        else if (next < 0)
          focus(active_nav);
      }
      return;
    }
    if (dy > 0 && main) {
      Rml::ElementList content;
      main->QuerySelectorAll(content, "[data-focus]");
      content.erase(std::remove_if(content.begin(), content.end(),
                                   [&](Rml::Element *element) {
                                     return ancestor_with_class(
                                                element, "local-tabs") ||
                                            ancestor_with_class(
                                                element, "mobile-local-tabs");
                                   }),
                    content.end());
      focus(geometric_target(current, content));
    }
    return;
  }

  if (main && is_descendant_of(current, main)) {
    Rml::ElementList content;
    main->QuerySelectorAll(content, "[data-focus]");
    content.erase(
        std::remove_if(content.begin(), content.end(),
                       [](Rml::Element *element) {
                         return !is_visible_focus(element);
                       }),
        content.end());
    if (Rml::Element *best = geometric_target(current, content))
      focus(best);
    else if ((!compact_horizontal_nav && dx < 0) ||
             (compact_horizontal_nav && dy > 0))
      focus(active_nav);
    return;
  }

  focus(active_nav);
}

void GubsyApp::ActivateFocus() {
  Rml::Element *focus = context_->GetFocusElement();
  if (!focus)
    return;
  if (auto *select = dynamic_cast<Rml::ElementFormControlSelect *>(focus)) {
    if (select->IsSelectBoxVisible())
      select->HideSelectBox();
    else
      select->ShowSelectBox();
    return;
  }
  if (auto *input = dynamic_cast<Rml::ElementFormControlInput *>(focus);
      input && input->GetAttribute<Rml::String>("type", "") == "checkbox") {
    if (input->HasAttribute("checked"))
      input->RemoveAttribute("checked");
    else
      input->SetAttribute("checked", "checked");
    return;
  }
  focus->Click();
}

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

void GubsyApp::Render() {
  if (!document_)
    return;
  RenderChrome();
  Rml::String focused_action;
  if (!pending_focus_action_.empty())
    focused_action = pending_focus_action_;
  else if (Rml::Element *focused = action_element(context_->GetFocusElement()))
    focused_action = focused->GetAttribute<Rml::String>("data-action", "");
  if (Rml::Element *content = document_->GetElementById("screen-content"))
    content->SetInnerRML(BuildCurrentScreen());
  if (!focused_action.empty()) {
    Rml::ElementList candidates;
    document_->QuerySelectorAll(candidates, "[data-action]");
    for (Rml::Element *candidate : candidates) {
      if (candidate->GetAttribute<Rml::String>("data-action", "") ==
          focused_action) {
        candidate->Focus(true);
        break;
      }
    }
  }
  pending_focus_action_.clear();
  if (Rml::Element *modal = document_->GetElementById("modal-root")) {
    if (state_.modal.empty()) {
      modal->SetInnerRML("");
      modal->SetClass("visible", false);
    } else {
      const bool uninstall = state_.modal == "uninstall-plan";
      modal->SetInnerRML(
          std::string(
              R"(<div class="modal-card"><small>CONFIRM LOCAL CHANGE</small><h2>)") +
          (uninstall ? "Uninstall package graph?" : "Are you sure?") +
          (uninstall
               ? R"(</h2><p>Underground Rivers is required by Temple Weather and Pocket Expedition. Confirming removes all three, or cancel to preserve the installed graph.</p>)"
               : R"(</h2><p>This destructive demo action is guarded and remains local to the native prototype.</p>)") +
          R"(<div class="actions"><button data-focus data-action="cancel-modal" class="button">Cancel</button><button data-focus data-action="confirm-modal" class="button danger">Confirm</button></div></div>)");
      modal->SetClass("visible", true);
      if (Rml::Element *cancel =
              modal->QuerySelector("[data-action='cancel-modal']"))
        cancel->Focus(true);
    }
  }
  dirty_ = false;
}

void GubsyApp::RenderChrome() {
  if (!document_)
    return;
  if (Rml::Element *viewport = document_->GetElementById("viewport-size"))
    viewport->SetInnerRML(std::to_string(viewport_width_) + " × " +
                          std::to_string(viewport_height_));
  if (Rml::Element *title = document_->GetElementById("screen-title"))
    title->SetInnerRML(current_screen_name());

  struct NavEntry {
    const char *id;
    Destination destination;
  };
  constexpr NavEntry entries[]{
      {"play", Destination::Play},         {"players", Destination::Players},
      {"settings", Destination::Settings}, {"controls", Destination::Controls},
      {"progress", Destination::Progress}, {"mods", Destination::Mods}};
  for (const NavEntry &entry : entries) {
    if (Rml::Element *element =
            document_->GetElementById(std::string("nav-") + entry.id))
      element->SetClass("selected", state_.destination == entry.destination);
  }
}

std::string GubsyApp::BuildCurrentScreen() const {
  if (state_.provider_state != "Populated") {
    if (state_.provider_state == "Loading")
      return R"(<div class="state-surface provider-state"><small>PROVIDER REQUEST IN FLIGHT</small><h2>Loading game data…</h2><p>The fixed shell and navigation remain stable while this provider resolves.</p><div class="loading-bar"><i></i></div></div>)";
    if (state_.provider_state == "Empty")
      return R"(<div class="state-surface provider-state"><small>VALID EMPTY STATE</small><h2>Nothing here yet</h2><p>This provider returned an empty collection. Create the first item or return to another section.</p><button data-focus data-action="create-first" class="button primary">Create first item</button></div>)";
    return R"(<div class="state-surface provider-state error-state"><small>PROVIDER ERROR</small><h2>Game data could not be loaded</h2><p>The native shell remains usable. Retry this provider without rebuilding or losing the rest of the menu state.</p><button data-focus data-action="retry-provider" class="button primary">Retry</button></div>)";
  }
  switch (state_.destination) {
  case Destination::Play:
    switch (state_.play_view) {
    case PlayView::Lobby:
      return BuildPlayLobby();
    case PlayView::Quest:
      return BuildQuestPicker();
    case PlayView::Rules:
      return BuildRules();
    case PlayView::SessionMods:
      return BuildSessionMods();
    }
    break;
  case Destination::Players:
    return BuildPlayers();
  case Destination::Settings:
    return BuildSettings();
  case Destination::Controls:
    return BuildControls();
  case Destination::Progress:
    return BuildProgress();
  case Destination::Mods:
    return BuildMods();
  }
  return {};
}

std::string GubsyApp::BuildPlayLobby() const {
  std::ostringstream out;
  const bool continuing = state_.activity == "Continue expedition";
  const bool arena = state_.activity == "Arena run";
  const char *activity_label =
      continuing ? "CONTINUE QUEST"
                 : (arena ? "ARENA ACTIVITY" : "NEW EXPEDITION");
  const char *action_label =
      continuing ? "▶ Resume latest checkpoint"
                 : (arena ? "▶ Enter arena lobby" : "▶ Begin expedition");
  out << R"(<div class="mobile-local-tabs"><button data-focus data-action="show-setup" class=")"
      << (state_.party_pane ? "" : "selected")
      << R"(">Setup</button><button data-focus data-action="show-party" class=")"
      << (state_.party_pane ? "selected" : "")
      << R"(">Party · 1/4</button></div><div class="workspace lobby-workspace">)";
  out << R"(<section class="panel setup-pane )"
      << (state_.party_pane ? "phone-hidden" : "")
      << R"("><div class="quest-row"><img class="quest-art" src="../splonks-title.png"/><div><small>)"
      << activity_label << R"(</small><h2>)"
      << (arena ? "The Relay Pit" : state_.selected_quest) << R"(</h2><p>)"
      << (continuing ? "The Glass Caverns · "
                     : (arena ? "Round set · " : "Quest route · "))
      << (continuing ? state_.selected_checkpoint
                     : (arena ? "Competitive rules" : "Stage one"))
      << R"( · Vega</p></div><button data-focus data-action="play-quest" class="button compact">)"
      << (continuing ? "Checkpoint" : (arena ? "Arena" : "Quest"))
      << R"(</button></div>)";
  out << R"(<div class="option-row"><span><strong>Activity</strong><small>Splonks supplies a different session model for each activity</small></span>)";
  append_select(out, "play-value-activity", state_.activity,
                {"Continue expedition", "New expedition", "Arena run"});
  out << R"(</div>)";
  out << R"(<button data-focus data-action="play-quest" class="option-row command"><span><strong>)"
      << (continuing ? "Resume point"
                     : (arena ? "Arena rotation" : "Starting quest"))
      << R"(</strong><small>)"
      << (continuing
              ? state_.selected_checkpoint
              : (arena ? "Relay Pit · best of five" : state_.selected_quest))
      << R"(</small></span><b>)"
      << (continuing ? "CHOOSE CHECKPOINT ›" : "CHANGE ›")
      << R"(</b></button>)";
  out << R"(<div class="option-row"><span><strong>Play with</strong><small>Who may occupy the remaining slots</small></span>)";
  append_select(out, "play-value-access", state_.access,
                {"Solo", "Friends can join", "Invite only", "Public"});
  out << R"(</div><div class="option-row"><span><strong>Host using</strong><small>Automatic chooses the best available route</small></span>)";
  append_select(out, "play-value-host", state_.host,
                {"Automatic", "Host locally", "Dedicated relay"});
  out << R"(</div>)";
  out << R"(<button data-focus data-action="play-rules" class="option-row command"><span><strong>Expedition rules</strong><small>Standard · )"
      << map_value(state_.rule_values, "shared-lives") << " lives · ghost at "
      << map_value(state_.rule_values, "ghost-arrival")
      << R"(s</small></span><b>EDIT ALL ›</b></button>)";
  out << R"(<button data-focus data-action="play-mods" class="option-row command"><span><strong>Session mods</strong><small>7 active · differs from checkpoint</small></span><b>MANAGE ›</b></button>)";
  out << R"(<div class="actions"><button data-focus data-action="pause-preview" class="button">Pause preview</button><button data-focus data-action="start-session" class="button primary">)"
      << action_label << R"(</button></div></section>)";
  out << R"(<aside class="panel party-pane )"
      << (!state_.party_pane ? "phone-hidden" : "")
      << R"("><div class="panel-heading"><div><small>PLAYERS</small><h2>Your party</h2></div><b>OFFLINE</b></div><div class="player"><em>P1</em><span><strong>Moss</strong><small>Xbox Wireless Controller</small></span><b>READY</b></div>)";
  for (int i = 0; i < 3; ++i)
    out << R"(<button data-focus data-action="add-player" class="open-slot">+<span>Open slot<small>Invite a friend or add locally</small></span></button>)";
  out << R"(<div class="pair"><button data-focus data-action="copy-link" class="button">Invite / copy link</button><button data-focus data-action="find-games" class="button">Friends &amp; public games</button></div><dl class="summary"><dt>CONTENT</dt><dd>7 mods</dd><dt>RULESET</dt><dd>Standard</dd><dt>NETWORK</dt><dd>Automatic</dd></dl></aside></div>)";
  return out.str();
}

std::string GubsyApp::BuildQuestPicker() const {
  const bool continuing = state_.activity == "Continue expedition";
  const bool arena = state_.activity == "Arena run";
  struct Quest {
    const char *action;
    const char *name;
    const char *region;
    const char *stages;
  };
  constexpr Quest quests[]{
      {"quest-violet", "The Violet Reach", "Temple frontier · Moderate", "6"},
      {"quest-sunken", "The Sunken Road", "River caverns · Hard", "5"},
      {"quest-green", "A Green Beginning", "Valley underworks · Welcoming",
       "4"},
      {"quest-glass", "The Glass Pilgrim", "Crystal descent · Severe", "7"}};
  std::ostringstream out;
  out << R"(<div class="subview-header panel"><button data-focus data-action="play-lobby" class="button">‹ Back to lobby</button><div><small>SPLONKS QUEST PROVIDER</small><h2>)"
      << (continuing ? "Choose a resume point"
                     : (arena ? "Choose an arena rotation" : "Choose a quest"))
      << R"(</h2><p>)"
      << (continuing ? "Continue data selects a quest, its owning profile, and "
                       "a checkpoint inside that quest."
                     : (arena ? "Arena activities choose competitive maps and "
                                "round structure without campaign checkpoints."
                              : "A new expedition begins at stage one of the "
                                "selected quest with a fresh run manifest."))
      << R"(</p></div></div><div class="master-detail"><section class="panel master-list"><small>)"
      << (arena ? "ARENA ROTATIONS" : "EXPEDITIONS AND QUESTS")
      << R"(</small>)";
  for (const Quest &quest : quests) {
    out << R"(<button data-focus data-action=")" << quest.action
        << R"(" class=")"
        << selected_class(state_.selected_quest == quest.name, "list-row")
        << R"("><em>)" << quest.stages << R"(</em><span><strong>)" << quest.name
        << R"(</strong><small>)" << quest.region
        << R"(</small></span><b>QUEST</b></button>)";
  }
  out << R"(</section><article class="panel detail"><button data-focus data-action="play-lobby" class="mobile-back">‹ Quest choices</button><small>TEMPLE FRONTIER</small><h2>)"
      << state_.selected_quest
      << R"(</h2><p>Follow a fractured relay signal through flooded archives, fungal crossings, and a temple complex waking beneath the mountain.</p><div class="tag-row"><span>6 stages</span><span>Moderate</span><span>Latest checkpoint</span></div><h3>QUEST ROUTE</h3><div class="route">)";
  constexpr const char *stages[]{"North Pass",      "Mushroom Crossing",
                                 "Flooded Archive", "Temple Gate",
                                 "The Relay",       "Violet Core"};
  for (int i = 0; i < 6; ++i)
    out << R"(<div class=")" << (i < 4 ? "reached" : "") << R"("><b>)" << i + 1
        << R"(</b><span>)" << stages[i] << R"(</span></div>)";
  out << R"(</div><div class="context-box"><strong>)"
      << (continuing ? "Checkpoint payload"
                     : (arena ? "Arena manifest" : "New expedition manifest"))
      << R"(</strong><p>)"
      << (continuing
              ? "Inventory, quest flags, world seed, owning profile, and exact "
                "mod versions come from this checkpoint."
              : (arena ? "Map rotation, round rules, players, and session mods "
                         "are fixed when the arena lobby launches."
                       : "Quest, seed policy, expedition rules, players, and "
                         "session mods are recorded when the new run begins."))
      << R"(</p></div><button data-focus data-action="play-lobby" class="button primary">)"
      << (continuing ? "Use this checkpoint"
                     : (arena ? "Use this rotation" : "Start with this quest"))
      << R"(</button></article></div>)";
  return out.str();
}

std::string GubsyApp::BuildRules() const {
  struct Rule {
    const char *id;
    const char *name;
    const char *note;
    const char *kind;
    const char *source;
  };
  constexpr Rule rules[]{
      {"difficulty", "Expedition difficulty",
       "Overall danger and resource pressure.", "select", ""},
      {"variation", "Stage variation", "How authored rooms are remixed.",
       "select", ""},
      {"shared-lives", "Shared lives", "Continues available to the party.",
       "range", ""},
      {"starting-health", "Starting health",
       "Hearts granted at expedition start.", "range", ""},
      {"ghost-arrival", "Ghost arrival",
       "Seconds before the stage begins hunting.", "range", ""},
      {"shop-frequency", "Shop frequency", "Merchant room availability.",
       "select", ""},
      {"shortcuts", "Discovered shortcuts", "Allow unlocked route entrances.",
       "toggle", ""},
      {"treasury", "Shared treasury", "Combine party currency.", "toggle", ""},
      {"friendly-fire", "Friendly fire", "Players can damage each other.",
       "toggle", ""},
      {"lantern-fuel", "Lantern fuel",
       "Duration of carried expedition lanterns.", "select", "Old Lanterns"}};
  std::ostringstream out;
  out << R"(<div class="subview-header panel"><button data-focus data-action="play-lobby" class="button">‹ Back to lobby</button><div><small>SPLONKS SESSION RULES</small><h2>Expedition settings</h2><p>10 settings · 1 contributed by active mods.</p></div><button data-focus data-action="reset-rules" class="button">Reset mode defaults</button></div><div class="master-detail rules"><section class="panel master-list scroll-list"><div class="scroll-body">)";
  for (const Rule &rule : rules) {
    out << R"(<div class=")"
        << selected_class(state_.selected_rule == rule.name, "rule-row")
        << R"("><span><strong>)" << rule.name << R"(</strong><small>)"
        << rule.note << R"(</small>)";
    if (*rule.source)
      out << R"(<em>MOD · )" << rule.source << R"(</em>)";
    out << R"(</span><div class="rule-control">)";
    const std::string id = rule.id;
    if (id == "difficulty")
      append_select(out, "rule-value-difficulty",
                    map_value(state_.rule_values, id),
                    {"Relaxed", "Standard", "Dangerous", "Nightmare"},
                    "native-select rule-select");
    else if (id == "variation")
      append_select(out, "rule-value-variation",
                    map_value(state_.rule_values, id),
                    {"Quest-authored", "Remixed", "Fully random"},
                    "native-select rule-select");
    else if (id == "shop-frequency")
      append_select(
          out, "rule-value-shop-frequency", map_value(state_.rule_values, id),
          {"Rare", "Normal", "Frequent"}, "native-select rule-select");
    else if (id == "lantern-fuel")
      append_select(
          out, "rule-value-lantern-fuel", map_value(state_.rule_values, id),
          {"Scarce", "Standard", "Generous"}, "native-select rule-select");
    else if (std::string(rule.kind) == "range") {
      const int minimum = id == "ghost-arrival" ? 30 : 1;
      const int maximum = id == "ghost-arrival" ? 300 : 9;
      const int step = id == "ghost-arrival" ? 10 : 1;
      const std::string value = map_value(state_.rule_values, id);
      out << R"(<input data-focus data-action="rule-value-)" << id
          << R"(" class="native-range rule-range" type="range" min=")"
          << minimum << R"(" max=")" << maximum << R"(" step=")" << step
          << R"(" value=")" << value << R"("/><b id="rule-output-)" << id
          << R"(">)" << value
          << (id == "starting-health" ? " hearts"
              : id == "ghost-arrival" ? " sec"
                                      : "")
          << R"(</b>)";
    } else {
      const bool checked = id == "shortcuts"  ? state_.shortcuts
                           : id == "treasury" ? state_.shared_treasury
                                              : state_.friendly_fire;
      out << R"(<input data-focus data-action="rule-value-)" << id
          << R"(" class="native-toggle" type="checkbox" value="true")";
      if (checked)
        out << R"( checked="checked")";
      out << R"(/><b id="rule-output-)" << id << R"(">)" << on_off(checked)
          << R"(</b>)";
    }
    out << R"(</div></div>)";
  }
  std::string selected_value = "Configured";
  std::string selected_description =
      "This game-provided rule is serialized into the session manifest and "
      "synchronized before launch.";
  if (state_.selected_rule == "Discovered shortcuts") {
    selected_value = on_off(state_.shortcuts);
    selected_description =
        "Lets the party enter from previously discovered quest junctions. "
        "Disable it for a clean route beginning at stage one.";
  } else if (state_.selected_rule == "Shared treasury") {
    selected_value = on_off(state_.shared_treasury);
    selected_description =
        "Combines collected currency for the party instead of tracking a "
        "separate wallet per local or remote player.";
  } else if (state_.selected_rule == "Friendly fire") {
    selected_value = on_off(state_.friendly_fire);
    selected_description =
        "Controls whether attacks and hazards authored as player damage may "
        "affect other members of the party.";
  } else if (state_.selected_rule == "Shared lives")
    selected_value = map_value(state_.rule_values, "shared-lives");
  else if (state_.selected_rule == "Starting health")
    selected_value =
        map_value(state_.rule_values, "starting-health") + " hearts";
  else if (state_.selected_rule == "Ghost arrival")
    selected_value = map_value(state_.rule_values, "ghost-arrival") + " sec";
  else if (state_.selected_rule == "Expedition difficulty")
    selected_value = map_value(state_.rule_values, "difficulty");
  else if (state_.selected_rule == "Stage variation")
    selected_value = map_value(state_.rule_values, "variation");
  else if (state_.selected_rule == "Shop frequency")
    selected_value = map_value(state_.rule_values, "shop-frequency");
  else if (state_.selected_rule == "Lantern fuel")
    selected_value =
        map_value(state_.rule_values, "lantern-fuel") + " · Old Lanterns";
  out << R"(</div></section><aside class="panel detail rule-detail"><button data-focus data-action="play-lobby" class="mobile-back">‹ All rules</button><small>SELECTED RULE</small><h2 id="rule-detail-title">)"
      << state_.selected_rule << R"(</h2><p>)" << selected_description
      << R"(</p><div class="value-box"><small>CURRENT VALUE</small><strong id="rule-detail-current">)"
      << selected_value
      << R"(</strong></div><div class="impact"><h3>SESSION EFFECT</h3><div class="kv"><span>Activity</span><b>Continue expedition</b></div><div class="kv"><span>Applies to</span><b>Future stages</b></div><div class="kv"><span>Authority</span><b>Lobby host</b></div><div class="kv"><span>Synced</span><b>Before launch</b></div></div><p class="caption">The game supplies rule definitions. Gubsy supplies stable editor widgets, focus, serialization, and synchronization.</p></aside></div>)";
  return out.str();
}

std::string GubsyApp::BuildSessionMods() const {
  struct Mod {
    const char *name;
    const char *version;
    const char *note;
    bool active;
  };
  constexpr Mod mods[]{
      {"Base Content", "1.4.0", "Required root package", true},
      {"Old Lanterns", "1.3.1", "Needs update to 1.4.0", false},
      {"Underground Rivers", "2.2.0", "1 required dependency", true},
      {"Mycelium Below", "1.1.0", "Hot-safe runtime hooks", true},
      {"Brassline Grapple Kit", "0.8.4", "Applies next stage", true},
      {"Echoing Markets", "1.0.2", "2 required dependencies", true},
      {"Temple Weather", "3.0.0", "New-session recommended", true}};
  std::ostringstream out;
  const bool continuing = state_.activity == "Continue expedition";
  out << R"(<div class="subview-header panel session-mod-head"><button data-focus data-action="play-lobby" class="button">‹ Back to lobby</button><div><small>CURRENT LOBBY / CONTENT</small><h2>Session mods</h2><p>)"
      << (continuing
              ? "Checkpoint \"Temple safe room\" has a recorded package set."
              : "Choose content for this new session before launch.")
      << R"(</p></div><div class="local-tabs"><button data-focus data-action="session-current" class=")"
      << selected_class(!state_.session_mod_browse)
      << R"(">Current set</button><button data-focus data-action="session-browse" class=")"
      << selected_class(state_.session_mod_browse)
      << R"(">Browse &amp; add</button></div></div>)";
  if (state_.session_mod_browse) {
    struct Candidate {
      const char *name;
      const char *author;
      const char *note;
      const char *dependency;
    };
    constexpr Candidate candidates[]{
        {"Mycelium Below", "Root & Branch", "Worlds · Biomes · Co-op",
         "Underground Rivers will install"},
        {"Brassline Grapple Kit", "Clockwork Moss", "Mechanics · Traversal",
         "Cartographer's Desk installed"},
        {"Skybreak Caverns", "Gale Assembly", "Worlds · Sky",
         "No missing dependencies"},
        {"Ember Orchard", "Kindling Club", "Biome · Items",
         "Base Content installed"},
        {"Clockwork Descent", "Ratchet House", "Quest · Machines",
         "No missing dependencies"},
        {"Friendly Ghosts", "Pale Picnic", "Creatures · Co-op",
         "Base Content installed"},
        {"Campfire Stories", "Wayfarer", "Quests · Dialogue",
         "Old Lanterns update required"},
        {"Relay Races", "Split Second", "Mode · Co-op",
         "Networking 1.2 installed"}};
    out << R"(<div class="catalog-tools"><input data-focus data-action="search-catalog" class="search-field" type="text" value=")"
        << escape_attribute(state_.mod_filter)
        << R"(" placeholder="Search compatible session content…"/><label class="check-control"><input data-focus data-action="compatible-value" class="native-toggle" type="checkbox" value="true")";
    if (state_.compatible_only)
      out << R"( checked="checked")";
    out << R"(/><span>Compatible only<small>Resolve against this lobby</small></span></label></div><div class="master-detail"><section class="panel master-list scroll-list catalog-list"><div class="scroll-body"><small>AVAILABLE FOR THIS SESSION</small>)";
    for (const Candidate &candidate : candidates) {
      if (!contains_ci(candidate.name, state_.mod_filter) &&
          !contains_ci(candidate.author, state_.mod_filter) &&
          !contains_ci(candidate.note, state_.mod_filter))
        continue;
      const size_t image_index = static_cast<size_t>(&candidate - candidates);
      out << R"(<button data-focus data-action="session-mod-)" << candidate.name
          << R"(" class=")"
          << selected_class(state_.selected_mod == candidate.name,
                            "catalog-row")
          << R"("><img class="mod-thumb" src="../mods/)"
          << mod_image(image_index) << R"("/><span><strong>)" << candidate.name
          << R"(</strong><small>)" << candidate.note << " · by "
          << candidate.author << R"(</small><em>)" << candidate.dependency
          << R"(</em></span><b>SESSION READY</b><i></i></button>)";
    }
    out << R"(</div></section><aside class="panel detail mod-detail"><img class="hero-art" src="../mods/sheet-1-0.png"/><small>ADD TO CURRENT SESSION</small><h2>)"
        << state_.selected_mod
        << R"(</h2><p>Review the complete solved change plan here; installation and activation are one operation from the lobby.</p><div class="impact"><h3>SESSION CHANGE PLAN</h3><div class="kv"><span>Package</span><b>Install latest compatible</b></div><div class="kv"><span>Dependencies</span><b>1 automatic</b></div><div class="kv"><span>Activation</span><b>This lobby</b></div><div class="kv"><span>Runtime policy</span><b>Safe before launch</b></div></div><div class="dependency"><h3>AUTOMATIC DEPENDENCIES</h3><div><span>↳ Base Content v1.4.0</span><b>INSTALLED</b></div><div><span>↳ Underground Rivers v2.2.0</span><b>WILL INSTALL</b></div></div><button data-focus data-action="install-add-session" class="button primary">Install dependencies &amp; add to session</button><button data-focus data-action="session-current" class="button">Cancel</button></aside></div>)";
    return out.str();
  }
  if (continuing)
    out << R"(<div class="manifest-banner"><span><strong>Checkpoint mod set differs from this lobby</strong><small>1 missing · 1 inactive · 2 extra</small></span><button data-focus data-action="sync-manifest" class="button primary">Install &amp; sync checkpoint set</button></div>)";
  else
    out << R"(<div class="manifest-banner clean"><span><strong>New-session manifest</strong><small>Changes here become the recorded starting set.</small></span><b>7 ACTIVE PACKAGES</b></div>)";
  out << R"(<div class="master-detail"><section class="panel master-list scroll-list"><div class="scroll-body">)";
  for (const Mod &mod : mods) {
    const size_t image_index = static_cast<size_t>(&mod - mods);
    out << R"(<button data-focus data-action="session-mod-)" << mod.name
        << R"(" class=")"
        << selected_class(state_.selected_mod == mod.name, "mod-row")
        << R"("><img class="mod-thumb" src="../mods/)" << mod_image(image_index)
        << R"("/><span><strong>)" << mod.name << R"(</strong><small>v)"
        << mod.version << " · " << mod.note << R"(</small></span><b class=")"
        << (mod.active ? "active" : "") << R"(">)"
        << (mod.active ? "ACTIVE" : "UPDATE → 1.4.0") << R"(</b></button>)";
  }
  out << R"(</div></section><aside class="panel detail mod-detail"><button data-focus data-action="play-lobby" class="mobile-back">‹ All session mods</button><div class="hero-art"></div><small>SESSION CONTENT</small><h2>)"
      << state_.selected_mod
      << R"(</h2><p>Review this package's version, runtime policy, dependencies, and dependents without leaving the session.</p>)";
  if (state_.selected_mod == "Old Lanterns")
    out << R"(<div class="warning-box"><strong>Update required before use</strong><small>Installed v1.3.1 → compatible v1.4.0</small><button data-focus data-action="update-enable" class="button primary">Update &amp; enable</button></div>)";
  else
    out << R"(<div class="value-box"><small>CURRENT SESSION STATE</small><strong>ACTIVE · COMPATIBLE</strong></div>)";
  out << R"(<div class="dependency"><h3>Required dependencies</h3><div><span>↳ Base Content</span><b>INSTALLED</b></div><h3>Required by installed mods</h3><div><span>↑ Temple Weather</span><b>ACTIVE</b></div></div></aside></div>)";
  return out.str();
}

std::string GubsyApp::BuildPlayers() const {
  std::ostringstream out;
  out << R"(<div class="local-tabs"><button data-focus data-action="player-tab-Local players" class=")"
      << selected_class(state_.player_tab == "Local players")
      << R"(">Local players</button><button data-focus data-action="player-tab-Profiles" class=")"
      << selected_class(state_.player_tab == "Profiles")
      << R"(">Profiles</button><button data-focus data-action="player-tab-Devices" class=")"
      << selected_class(state_.player_tab == "Devices")
      << R"(">Devices</button><span>LB / RB change section</span></div>)";
  if (state_.player_tab == "Local players") {
    out << R"(<div class="master-detail"><section class="panel master-list"><div class="section-title"><span><small>LOCAL ROSTER</small><strong>2 / 4 players</strong></span><button data-focus data-action="add-player" class="button primary">+</button></div><button data-focus data-action="profile-moss" class="list-row selected"><em>P1</em><span><strong>Moss</strong><small>Xbox Wireless Controller</small></span><b>)"
        << (state_.player_ready ? "READY" : "NOT READY")
        << R"(</b></button><button data-focus data-action="add-player" class="list-row"><em>P2</em><span><strong>Open slot</strong><small>Invite or join locally</small></span><b>OPEN</b></button></section><aside class="panel detail"><small>PLAYER 1 DETAILS</small><h2>Moss</h2><div class="form-row"><span>Player profile<small>Persistent identity and history</small></span>)";
    append_select(out, "choice-value-player-profile",
                  map_value(state_.choice_values, "player-profile"),
                  {"Moss", "Vega", "Guest"});
    out << R"(</div><div class="form-row"><span>Bindings<small>Independent action map</small></span>)";
    append_select(out, "choice-value-player-bindings",
                  map_value(state_.choice_values, "player-bindings"),
                  {"Default Binds", "Arcade Binds", "Vehicle Binds"});
    out << R"(</div><div class="form-row"><span>Input tuning<small>Deadzone and response profile</small></span>)";
    append_select(out, "choice-value-player-tuning",
                  map_value(state_.choice_values, "player-tuning"),
                  {"Standard", "Precise", "Accessible", "Custom"});
    out << R"(</div><h3>ASSIGNED DEVICES · MANY ALLOWED</h3><div class="device-chip"><span>Xbox Wireless Controller<small>Gamepad · connected</small></span><button data-focus data-action="unassign" class="button">Remove</button></div><div class="device-chip"><span>T-LCM Pedals<small>Pedal axis set · optional</small></span><button data-focus data-action="unassign" class="button">Remove</button></div><div class="actions"><button data-focus data-action="assign-device" class="button">+ Assign device</button><button data-focus data-action="toggle-ready" class="button primary">)"
        << (state_.player_ready ? "Mark not ready" : "Mark ready")
        << R"(</button></div></aside></div>)";
  } else if (state_.player_tab == "Profiles") {
    struct Profile {
      const char *action;
      const char *initials;
      const char *name;
      const char *stats;
    };
    constexpr Profile profiles[]{
        {"profile-moss", "MO", "Moss", "38h 22m · 84 runs · 21 wins"},
        {"profile-vega", "VE", "Vega", "14h 11m · 31 runs · 4 wins"},
        {"profile-guest", "GU", "Guest", "Never · 0 runs · 0 wins"}};
    out << R"(<section class="panel profile-workspace"><div class="section-title"><span><small>PROFILE LIBRARY</small><strong>3 profiles</strong></span><button data-focus data-action="new-profile" class="button primary">+ New profile</button></div><div class="profile-cards">)";
    for (const Profile &profile : profiles)
      out << R"(<button data-focus data-action=")" << profile.action
          << R"(" class=")"
          << selected_class(state_.selected_profile == profile.name,
                            "profile-card")
          << R"("><b>)" << profile.initials << R"(</b><strong>)" << profile.name
          << R"(</strong><small>)" << profile.stats << R"(</small><em>)"
          << (state_.selected_profile == profile.name ? "ACTIVE" : "SELECT")
          << R"(</em></button>)";
    out << R"(</div><div class="profile-history"><small>PROFILE DATA IS NOT A SAVE</small><h2>)"
        << state_.selected_profile
        << R"('s history</h2><div class="stat-strip"><span><b>84</b>PLAYS</span><span><b>21</b>WINS</span><span><b>138</b>DEATHS</span><span><b>2.4M</b>SCORE</span><span><b>42</b>REPLAYS</span></div><p>Achievements, aggregate stats, unlocks, scores, and replays survive across campaigns and checkpoints.</p></div></section>)";
  } else {
    struct Device {
      const char *action;
      const char *name;
      const char *owner;
      const char *kind;
    };
    constexpr Device devices[]{
        {"device-xbox", "Xbox Wireless Controller", "Moss",
         "Gamepad · battery 82%"},
        {"device-keyboard", "Keyboard + Mouse", "Unassigned",
         "Keyboard / pointer"},
        {"device-dualsense", "DualSense Wireless Controller", "Unassigned",
         "Gamepad · USB"},
        {"device-flight", "T.16000M Flight Stick", "Unassigned",
         "Joystick · 16 buttons · 4 axes"},
        {"device-pedals", "T-LCM Pedals", "Moss", "Pedals · 3 axes"}};
    out << R"(<div class="master-detail"><section class="panel master-list scroll-list"><div class="scroll-body"><div class="section-title"><span><small>RECOGNIZED DEVICES</small><strong>5 connected</strong></span><button data-focus data-action="refresh-devices" class="button">↻ Refresh</button></div>)";
    for (const Device &device : devices)
      out << R"(<button data-focus data-action=")" << device.action
          << R"(" class=")"
          << selected_class(state_.selected_device == device.name, "device-row")
          << R"("><span><strong>)" << device.name << R"(</strong><small>)"
          << device.kind << R"(</small></span><b>)" << device.owner
          << R"(</b></button>)";
    out << R"(</div>)";
    out << R"(</section><aside class="panel detail"><small>DEVICE OWNERSHIP</small><h2>)"
        << state_.selected_device
        << R"(</h2><p>A device may be assigned to one local player; each player may own any number of devices.</p><div class="owner-list"><button data-focus data-action="unassign" class="owner"><span>○</span><b>Unassigned</b><small>Ignore this device for local gameplay</small></button><button data-focus data-action="assign-moss" class="owner selected"><span>P1</span><b>Moss</b><small>Uses Default Binds</small></button><button data-focus data-action="assign-vega" class="owner"><span>P2</span><b>Vega</b><small>Uses Arcade Binds</small></button></div><button data-focus data-action="identify-device" class="button">Identify / rumble device</button></aside></div>)";
  }
  return out.str();
}

std::string GubsyApp::BuildSettings() const {
  std::ostringstream out;
  const char *tabs[]{"Display", "Audio", "Accessibility", "Gameplay"};
  out << R"(<div class="local-tabs">)";
  for (const char *tab : tabs)
    out << R"(<button data-focus data-action="settings-tab-)" << tab
        << R"(" class=")" << selected_class(state_.settings_tab == tab)
        << R"(">)" << tab << R"(</button>)";
  out << R"(<span>LB / RB change category</span></div><div class="master-detail settings-workspace"><section class="panel master-list scroll-list"><div class="scroll-body"><div class="section-title"><span><small>)"
      << state_.settings_tab << R"(</small><strong>)";
  struct Setting {
    const char *id;
    const char *name;
    const char *note;
    const char *value;
  };
  std::vector<Setting> settings;
  if (state_.settings_tab == "Display")
    settings = {
        {"fullscreen", "Fullscreen", "Use the entire selected display", "ON"},
        {"resolution", "Display resolution",
         "Output resolution for this display", "1920 × 1080⌄"},
        {"render-scale", "Render scale", "Internal 3D resolution", "100%⌄"},
        {"frame-cap", "Frame cap", "Maximum simulation frames per second",
         "144 FPS⌄"},
        {"brightness", "Brightness", "Fine-tune scene visibility", "64%"},
        {"vsync", "Vertical synchronization", "Avoid tearing when supported",
         "Adaptive⌄"}};
  else if (state_.settings_tab == "Audio")
    settings = {
        {"master-volume", "Master volume", "Final output gain", "82%"},
        {"music-volume", "Music volume", "Dynamic score and menus", "64%"},
        {"effects-volume", "Effects volume", "World and combat effects", "90%"},
        {"dialogue-volume", "Dialogue volume", "Spoken character audio",
         "100%"},
        {"dynamic-range", "Dynamic range", "Balance quiet and loud sounds",
         "Night⌄"},
        {"output-device", "Output device", "Active system audio sink",
         "Default⌄"}};
  else if (state_.settings_tab == "Accessibility")
    settings = {
        {"text-scale", "Text scale", "Interface and subtitle text", "125%⌄"},
        {"high-contrast", "High contrast focus",
         "Thicker focus and state markers", "ON"},
        {"reduce-motion", "Reduce motion", "Replace movement with fades",
         "OFF"},
        {"color-filter", "Color vision filter",
         "Transform critical color pairs", "Deuteranopia⌄"},
        {"hold-assists", "Hold input assists",
         "Convert repeated holds to toggles", "ON"},
        {"narration", "Menu narration", "Read focus changes and descriptions",
         "OFF"}};
  else
    settings = {{"tutorials", "Context tutorials",
                 "Show relevant mechanic reminders", "Smart⌄"},
                {"camera-shake", "Camera shake",
                 "Impact-driven camera movement", "35%"},
                {"auto-pause", "Pause when disconnected",
                 "Pause local sessions on device loss", "ON"},
                {"checkpoint-hints", "Checkpoint hints",
                 "Show route and recovery guidance", "ON"},
                {"speedrun-timer", "Speedrun timer",
                 "Persistent run split overlay", "OFF"},
                {"telemetry", "Anonymous telemetry",
                 "Share performance-only diagnostics", "OFF"}};
  out << settings.size()
      << R"( options</strong></span><b>SAVED LOCALLY</b></div>)";
  auto value_for = [&](const std::string &id) {
    const auto it = state_.setting_values.find(id);
    return it == state_.setting_values.end() ? std::string{} : it->second;
  };
  auto is_toggle = [](const std::string &id) {
    return id == "fullscreen" || id == "high-contrast" ||
           id == "reduce-motion" || id == "hold-assists" || id == "narration" ||
           id == "auto-pause" || id == "checkpoint-hints" ||
           id == "speedrun-timer" || id == "telemetry";
  };
  auto is_range = [](const std::string &id) {
    return id == "render-scale" || id == "brightness" ||
           id == "master-volume" || id == "music-volume" ||
           id == "effects-volume" || id == "dialogue-volume" ||
           id == "camera-shake";
  };
  auto options_for = [](const std::string &id) -> std::vector<std::string> {
    if (id == "resolution")
      return {"1280 × 720", "1920 × 1080", "2560 × 1440", "3840 × 2160"};
    if (id == "frame-cap")
      return {"60 FPS", "120 FPS", "144 FPS", "Unlimited"};
    if (id == "vsync")
      return {"Off", "On", "Adaptive"};
    if (id == "dynamic-range")
      return {"Night", "Medium", "Wide"};
    if (id == "output-device")
      return {"Default", "HDMI", "USB Headset"};
    if (id == "text-scale")
      return {"100%", "125%", "150%", "200%"};
    if (id == "color-filter")
      return {"Off", "Deuteranopia", "Protanopia", "Tritanopia"};
    if (id == "tutorials")
      return {"Off", "Smart", "Always"};
    return {};
  };
  std::string selected_id = "fullscreen";
  for (const Setting &setting : settings) {
    const std::string id = setting.id;
    const std::string value = value_for(id);
    if (state_.selected_setting == setting.name)
      selected_id = id;
    out << R"(<div id="setting-row-)" << id << R"(" data-action="setting-)"
        << setting.name << R"(" class=")"
        << selected_class(state_.selected_setting == setting.name,
                          "setting-row")
        << R"("><span><strong>)" << setting.name << R"(</strong><small>)"
        << setting.note << R"(</small></span><div class="setting-control">)";
    if (is_toggle(id)) {
      out << R"(<input data-focus data-action="setting-value-)" << id
          << R"(" data-setting-name=")" << setting.name
          << R"(" class="setting-toggle" type="checkbox" value="true")";
      if (value == "true")
        out << R"( checked="checked")";
      out << R"(/><b id="value-)" << id << R"(">)"
          << format_setting_value(id, value) << R"(</b>)";
    } else if (is_range(id)) {
      const int minimum = id == "render-scale" ? 50 : 0;
      const int maximum = id == "render-scale" ? 150 : 100;
      out << R"(<input data-focus data-action="setting-value-)" << id
          << R"(" data-setting-name=")" << setting.name
          << R"(" class="setting-range" type="range" min=")" << minimum
          << R"(" max=")" << maximum << R"(" step="1" value=")"
          << escape_attribute(value) << R"("/><b id="value-)" << id << R"(">)"
          << format_setting_value(id, value) << R"(</b>)";
    } else {
      out << R"(<select data-focus data-action="setting-value-)" << id
          << R"(" data-setting-name=")" << setting.name
          << R"(" class="setting-select">)";
      for (const std::string &option : options_for(id)) {
        out << R"(<option value=")" << escape_attribute(option) << R"(")";
        if (option == value)
          out << R"( selected="selected")";
        out << R"(>)" << option << R"(</option>)";
      }
      out << R"(</select>)";
    }
    out << R"(</div></div>)";
  }
  const std::string selected_value = value_for(selected_id);
  out << R"(</div></section><aside class="panel detail setting-detail"><small>SELECTED SETTING</small><h2 id="setting-detail-title">)"
      << state_.selected_setting
      << R"(</h2><p>Change the native control directly. Values stay in local prototype state and the game-facing provider would validate and commit them.</p><div class="value-box setting-current"><small>CURRENT VALUE</small><strong id="setting-detail-current">)"
      << format_setting_value(selected_id, selected_value)
      << R"(</strong></div><div class="impact"><h3>SETTING CONTEXT</h3><div class="kv"><span>Scope</span><b>Current machine</b></div><div class="kv"><span>Applies</span><b>Immediately</b></div><div class="kv"><span>Profile</span><b>Vega</b></div><div class="kv"><span>Restart</span><b>Not required</b></div></div><div class="detail-actions"><button data-focus data-action="apply-setting" class="button primary">Apply )"
      << state_.settings_tab
      << R"( settings</button><button data-focus data-action="reset-category" class="button">Reset category</button></div></aside></div>)";
  return out.str();
}

std::string GubsyApp::BuildControls() const {
  std::ostringstream out;
  const char *tabs[]{"Bindings", "Devices", "Input tuning"};
  out << R"(<div class="local-tabs">)";
  for (const char *tab : tabs)
    out << R"(<button data-focus data-action="controls-tab-)" << tab
        << R"(" class=")" << selected_class(state_.controls_tab == tab)
        << R"(">)" << tab << R"(</button>)";
  out << R"(<span>LB / RB change section</span></div>)";
  if (state_.controls_tab == "Bindings") {
    struct Action {
      const char *name;
      const char *type;
      const char *summary;
    };
    constexpr Action actions[]{
        {"Menu Up", "Digital action · Menu", "D-Pad Up"},
        {"Menu Down", "Digital action · Menu", "D-Pad Down"},
        {"Menu Left", "Digital action · Menu", "D-Pad Left"},
        {"Menu Right", "Digital action · Menu", "D-Pad Right"},
        {"Activate", "Digital action · Menu", "Gamepad A"},
        {"Cancel", "Digital action · Menu", "Gamepad B"},
        {"Move", "Vector2 action · Player", "Left Stick"},
        {"Look", "Vector2 action · Player", "Right Stick"},
        {"Accelerate", "Scalar action · Vehicle", "Right Trigger"},
        {"Brake", "Scalar action · Vehicle", "Left Trigger"}};
    out << R"(<div class="toolbar"><input data-focus data-action="filter-actions" class="search-field" type="text" value=")"
        << escape_attribute(state_.control_filter)
        << R"(" placeholder="Filter actions…"/>)";
    append_select(out, "choice-value-bind-profile",
                  map_value(state_.choice_values, "bind-profile"),
                  {"Default Binds", "Arcade Binds", "Vehicle Binds"});
    out << R"(<button data-focus data-action="new-binds" class="button">+ New</button><button data-focus data-action="rename-binds" class="button">Rename</button><button data-focus data-action="reset-binds" class="button">Reset</button><button data-focus data-action="delete-binds" class="button danger">Delete</button></div><div class="master-detail"><section class="panel master-list scroll-list"><div class="scroll-body">)";
    for (const Action &action : actions) {
      if (!contains_ci(action.name, state_.control_filter) &&
          !contains_ci(action.type, state_.control_filter) &&
          !contains_ci(action.summary, state_.control_filter))
        continue;
      out << R"(<button data-focus data-action="action-)" << action.name
          << R"(" class=")"
          << selected_class(state_.selected_action == action.name, "action-row")
          << R"("><span><strong>)" << action.name << R"(</strong><small>)"
          << action.type << R"(</small></span><b>)" << action.summary
          << R"(</b></button>)";
    }
    out << R"(</div>)";
    out << R"(</section><aside class="panel detail binding-detail"><small>SELECTED LOGICAL ACTION</small><h2>)"
        << state_.selected_action
        << R"(</h2><p>Logical actions accept multiple physical controls. A binding may listen for input or explicitly browse a device, control, direction, conversion, and qualifier.</p><div class="binding-row"><em>1</em><span><strong>D-Pad Up</strong><small>Xbox Wireless Controller · digital</small></span><button data-focus data-action="capture-binding" class="text-button">Listen</button><button data-focus data-action="choose-binding" class="text-button">Choose</button><button data-focus data-action="remove-binding" class="remove">×</button></div><div class="binding-row"><em>2</em><span><strong>Keyboard W</strong><small>Keyboard + Mouse · digital</small></span><button data-focus data-action="capture-binding" class="text-button">Listen</button><button data-focus data-action="choose-binding" class="text-button">Choose</button><button data-focus data-action="remove-binding" class="remove">×</button></div><div class="binding-row"><em>3</em><span><strong>Right Trigger → Button</strong><small>Threshold 0.62 · rising edge · explicit conversion</small></span><button data-focus data-action="capture-binding" class="text-button">Listen</button><button data-focus data-action="choose-binding" class="text-button">Choose</button><button data-focus data-action="remove-binding" class="remove">×</button></div><div class="actions"><button data-focus data-action="capture-binding" class="button primary">)"
        << (state_.capture_mode ? "Listening…" : "+ Listen for input")
        << R"(</button><button data-focus data-action="choose-binding" class="button">Browse controls…</button></div><div class="conversion-panel"><h3>EXPLICIT BINDING EDITOR</h3><div class="form-row"><span>Device<small>Any, owned device, or exact instance</small></span>)";
    append_select(out, "choice-value-binding-device",
                  map_value(state_.choice_values, "binding-device"),
                  {"Any owned device", "Xbox Wireless Controller",
                   "T-LCM Pedals", "T.16000M Flight Stick"});
    out << R"(</div><div class="form-row"><span>Physical control<small>Axes, hats, buttons, keys, gestures</small></span>)";
    append_select(out, "choice-value-binding-control",
                  map_value(state_.choice_values, "binding-control"),
                  {"Right Trigger", "Left Trigger", "Button South", "Axis 0"});
    out << R"(</div><div class="form-row"><span>Conversion<small>Scalar input drives a digital action</small></span>)";
    append_select(out, "choice-value-binding-conversion",
                  map_value(state_.choice_values, "binding-conversion"),
                  {"Axis → Button", "Axis → Scalar", "Button → Scalar",
                   "Raw passthrough"});
    out << R"(</div><div class="form-row"><span>Threshold / edge<small>Qualify activation without losing the source</small></span>)";
    append_select(
        out, "choice-value-binding-threshold",
        map_value(state_.choice_values, "binding-threshold"),
        {"0.25 · Rising", "0.50 · Rising", "0.62 · Rising", "0.75 · Falling"});
    out << R"(</div></div></aside></div>)";
  } else if (state_.controls_tab == "Devices") {
    struct Device {
      const char *action;
      const char *name;
      const char *info;
      const char *owner;
    };
    constexpr Device devices[]{
        {"device-keyboard", "Keyboard + Mouse", "Keyboard / pointer · Built in",
         "Moss"},
        {"device-xbox", "Xbox Wireless Controller", "Gamepad · Bluetooth · 80%",
         "Moss"},
        {"device-dualsense", "DualSense Wireless Controller", "Gamepad · USB",
         "UNASSIGNED"},
        {"device-flight", "T.16000M Flight Stick",
         "Joystick · USB · 16 buttons", "UNASSIGNED"},
        {"device-pedals", "T-LCM Pedals", "Pedals · USB · 3 axes", "Moss"}};
    out << R"(<div class="toolbar"><span class="toolbar-title"><small>RECOGNIZED DEVICES</small>5 connected · one player may own many</span><button data-focus data-action="refresh-devices" class="button">↻ Rescan</button></div><div class="master-detail"><section class="panel master-list scroll-list"><div class="scroll-body">)";
    for (const Device &device : devices)
      out << R"(<button data-focus data-action=")" << device.action
          << R"(" class=")"
          << selected_class(state_.selected_device == device.name, "device-row")
          << R"("><span><strong>)" << device.name << R"(</strong><small>)"
          << device.info << R"(</small></span><b>)" << device.owner
          << R"(</b></button>)";
    out << R"(</div>)";
    out << R"(</section><aside class="panel detail device-detail"><small>RECOGNIZED INPUT DEVICE</small><h2>)"
        << state_.selected_device
        << R"(</h2><p>Assign this device to one local player. A player can combine a keyboard, wheel, pedals, flight stick, macro pad, and gamepad.</p><div class="owner-list"><button data-focus data-action="unassign" class="owner"><span>○</span><b>Unassigned</b><small>Ignore for local gameplay</small></button><button data-focus data-action="assign-moss" class="owner selected"><span>P1</span><b>Moss</b><small>Uses Default Binds</small></button><button data-focus data-action="assign-vega" class="owner"><span>P2</span><b>Vega</b><small>Uses Arcade Binds</small></button></div><div class="explorer"><div class="section-title"><span><small>LIVE INPUT EXPLORER</small><strong>Actuate anything on the device</strong></span><button data-focus data-action="identify-device" class="button">Identify</button></div><div class="raw-control"><span>)"
        << state_.raw_input_name << R"(</span><b>)" << std::fixed
        << std::setprecision(3) << state_.raw_input_value
        << R"(</b><em><i style="width:)"
        << static_cast<int>(state_.raw_input_value * 100.0f)
        << R"(%;"></i></em></div><div class="raw-control"><span>Button 1 · South</span><b>UP</b><em><i style="width:0%;"></i></em></div><div class="raw-control"><span>Hat 0 · D-Pad</span><b>UP-RIGHT</b><em><i style="width:66%;"></i></em></div><p>This raw view reveals names and ranges exactly as Gubsy recognizes unusual hardware.</p></div></aside></div>)";
  } else {
    out << R"(<div class="master-detail tuning-workspace"><section class="panel master-list scroll-list"><div class="scroll-body"><div class="section-title"><span><small>INPUT PROFILE</small><strong>Standard</strong></span><b>Xbox Wireless Controller</b></div>)";
    auto tuning_range = [&](const char *id, const char *name,
                            const char *note) {
      const std::string value = map_value(state_.tuning_values, id);
      out << R"(<div class="tuning-row"><span><strong>)" << name
          << R"(</strong><small>)" << note
          << R"(</small></span><input data-focus data-action="tuning-value-)"
          << id
          << R"(" class="native-range tuning-range" type="range" min="0" max="100" step="1" value=")"
          << value << R"("/><b id="tuning-output-)" << id << R"(">)" << value
          << R"(%</b></div>)";
    };
    tuning_range("look-sensitivity", "Look sensitivity",
                 "Horizontal and vertical camera speed");
    tuning_range("stick-deadzone", "Stick deadzone",
                 "Ignore small stick movement near center");
    tuning_range("vibration", "Vibration strength", "Controller rumble output");
    tuning_range("trigger-deadzone", "Trigger deadzone",
                 "Minimum pedal or trigger travel");
    out << R"(<div class="form-row"><span>Response curve<small>Stick magnitude transformation</small></span>)";
    append_select(out, "tuning-value-response-curve",
                  map_value(state_.tuning_values, "response-curve"),
                  {"Linear", "Smooth", "Aggressive", "Custom"});
    out << R"(</div><div class="form-row"><span>Invert Y-axis<small>Reverse vertical camera movement</small></span><div class="toggle-with-label"><input data-focus data-action="tuning-value-invert-y" class="native-toggle" type="checkbox" value="true")";
    if (map_value(state_.tuning_values, "invert-y") == "true")
      out << R"( checked="checked")";
    out << R"(/><b id="tuning-output-invert-y">)"
        << (map_value(state_.tuning_values, "invert-y") == "true" ? "ON"
                                                                  : "OFF")
        << R"(</b></div></div><button data-focus data-action="reset-tuning" class="button">Reset input profile</button></div></section><aside class="panel detail response-preview"><small>DEVICE RESPONSE</small><h2>Live response</h2><div class="response-circle"><i></i></div><dl class="summary"><dt>Device</dt><dd>Xbox Wireless Controller</dd><dt>Deadzone</dt><dd id="tuning-summary-stick-deadzone">)"
        << map_value(state_.tuning_values, "stick-deadzone")
        << R"(%</dd><dt>Vibration</dt><dd id="tuning-summary-vibration">)"
        << map_value(state_.tuning_values, "vibration")
        << R"(%</dd><dt>Curve</dt><dd id="tuning-summary-response-curve">)"
        << map_value(state_.tuning_values, "response-curve")
        << R"(</dd></dl><p>Move the device to compare raw and qualified output.</p></aside></div>)";
  }
  return out.str();
}

std::string GubsyApp::BuildProgress() const {
  struct Campaign {
    const char *id;
    const char *name;
    const char *quest;
    const char *time;
    const char *state;
  };
  constexpr Campaign campaigns[]{
      {"The Glass Caverns", "The Glass Caverns", "Moss · Temple Depths · 42%",
       "12h 48m", "READY"},
      {"A Quiet Beginning", "A Quiet Beginning", "Vega · Green Valley · 11%",
       "3h 06m", "READY"},
      {"Old Expedition", "Old Expedition", "Moss · Version 0.7 data", "8h 19m",
       "INCOMPATIBLE"}};
  std::ostringstream out;
  out << R"(<div class="toolbar"><span class="toolbar-title"><small>GAME-PROVIDED PROGRESSION</small>3 campaigns · automatic checkpoints</span><button data-focus data-action="new-campaign" class="button primary">+ Choose a new quest</button></div><div class="master-detail"><section class="panel master-list scroll-list"><div class="scroll-body"><small>CAMPAIGNS</small>)";
  for (const Campaign &campaign : campaigns)
    out << R"(<button data-focus data-action="campaign-)" << campaign.id
        << R"(" class=")"
        << selected_class(state_.selected_campaign == campaign.name,
                          "campaign-row")
        << R"("><img class="quest-art" src="../splonks-title.png"/><span><strong>)"
        << campaign.name << R"(</strong><small>)" << campaign.quest
        << R"(</small></span><em>)" << campaign.time << R"(</em><b>)"
        << campaign.state << R"(</b></button>)";
  out << R"(</div>)";
  out << R"(</section><aside class="panel detail progress-detail"><small>CAMPAIGN · PLAY</small><h2>)"
      << state_.selected_campaign
      << R"(</h2><div class="owner-banner"><b>MO</b><span><strong>ASSOCIATED PROFILE</strong><small>Moss · ownership is recorded by this progression provider</small></span></div><div class="impact"><h3>RECORDED MOD SET · 7 PACKAGES</h3><div class="kv"><span>Base Content</span><b>v1.4.0</b></div><div class="kv"><span>Cartographer's Desk</span><b>v0.8.2</b></div><div class="kv"><span>Old Lanterns</span><b>Update available</b></div><div class="kv"><span>Manifest hash</span><b>8cc91a</b></div><button data-focus data-action="inspect-manifest" class="button">Inspect exact package set</button></div><div class="checkpoint-history"><h3>CHECKPOINT HISTORY</h3><button data-focus data-action="checkpoint-temple"><span><strong>Temple safe room</strong><small>Today, 06:52 · Stage 4</small></span><b>RESUME POINT</b></button><button data-focus data-action="checkpoint-flooded"><span><strong>Flooded archive</strong><small>Yesterday · Stage 3</small></span><b>BACKUP</b></button><button data-focus data-action="checkpoint-crossing"><span><strong>Mushroom crossing</strong><small>Aug 24 · Stage 2</small></span><b>BACKUP</b></button></div><div class="actions"><button data-focus data-action="resume-campaign" class="button primary">▶ Resume campaign</button><button data-focus data-action="export-campaign" class="button">Export</button><button data-focus data-action="delete-campaign" class="button danger">Delete</button></div></aside></div>)";
  return out.str();
}

std::string GubsyApp::BuildMods() const {
  std::ostringstream out;
  out << R"(<div class="local-tabs"><button data-focus data-action="mods-tab-Installed" class=")"
      << selected_class(state_.mods_tab == "Installed")
      << R"(">Installed</button><button data-focus data-action="mods-tab-Browse catalog" class=")"
      << selected_class(state_.mods_tab == "Browse catalog")
      << R"(">Browse catalog</button><span>LB / RB change section</span></div>)";
  if (state_.mods_tab == "Installed") {
    struct Mod {
      const char *action;
      const char *name;
      const char *author;
      const char *version;
      const char *state;
    };
    constexpr Mod mods[]{
        {"mod-base", "Base Content", "Gubsy", "v1.4.0", "COMPATIBLE"},
        {"mod-cartographer", "Cartographer's Desk", "Yarrow", "v0.8.2",
         "COMPATIBLE"},
        {"mod-pocket", "Pocket Expedition", "Mossbyte", "v0.9.1", "COMPATIBLE"},
        {"mod-lanterns", "Old Lanterns", "Palmeret", "v1.3.1", "UPDATE"},
        {"mod-rivers", "Underground Rivers", "Bathysphere", "v2.2.0",
         "COMPATIBLE"},
        {"mod-mycelium", "Mycelium Below", "Root & Branch", "v1.1.0",
         "COMPATIBLE"},
        {"mod-weather", "Temple Weather", "Northglass", "v3.0.0",
         "COMPATIBLE"}};
    out << R"(<div class="toolbar"><span class="toolbar-title"><small>ON THIS DEVICE</small>7 installed · game build 1.4</span><button data-focus data-action="refresh-mods" class="button">↻ Refresh</button></div><div class="master-detail"><section class="panel master-list scroll-list"><div class="scroll-body">)";
    for (const Mod &mod : mods) {
      const size_t image_index = static_cast<size_t>(&mod - mods);
      out << R"(<button data-focus data-action="installed-mod-)" << mod.name
          << R"(" class=")"
          << selected_class(state_.selected_mod == mod.name, "mod-row")
          << R"("><img class="mod-thumb" src="../mods/)"
          << mod_image(image_index) << R"("/><span><strong>)" << mod.name
          << R"(</strong><small>by )" << mod.author << R"( · )" << mod.version
          << R"(</small></span><b class=")"
          << (std::string(mod.state) == "UPDATE" ? "warning" : "active")
          << R"(">)" << mod.state << R"(</b></button>)";
    }
    out << R"(</div>)";
    out << R"(</section><aside class="panel detail mod-detail"><img class="hero-art" src="../mods/sheet-1-4.png"/><small>INSTALLED PACKAGE</small><h2>)"
        << state_.selected_mod
        << R"(</h2><p>Game-ready content with authored rooms, mechanics, and data supplied through the Gubsy package graph.</p><div class="form-row"><span>Selected version<small>Changing versions re-solves dependencies</small></span>)";
    append_select(out, "choice-value-mod-version",
                  map_value(state_.choice_values, "mod-version"),
                  {"v1.2.0", "v1.3.1", "v1.4.0", "Latest compatible"});
    out << R"(</div><div class="dependency"><h3>REQUIRES</h3><div><span>↳ Base Content ≥ 1.4</span><b>INSTALLED</b></div><div><span>↳ Underground Rivers ≥ 2.0</span><b>INSTALLED</b></div><h3>REQUIRED BY</h3><div><span>↑ Temple Weather</span><b>ACTIVE</b></div><div><span>↑ Pocket Expedition</span><b>ACTIVE</b></div></div><div class="warning-box"><strong>Cascading uninstall is guarded</strong><small>Removing this package also affects two installed dependents. Gubsy computes and presents the full change plan before mutation.</small></div><div class="actions mod-actions"><button data-focus data-action="update-mod" class="button primary">Update</button><button data-focus data-action="open-mod-folder" class="button">Open files</button><button data-focus data-action="uninstall-plan" class="button danger">Uninstall…</button></div></aside></div>)";
  } else {
    struct Catalog {
      const char *action;
      const char *name;
      const char *author;
      const char *category;
      const char *downloads;
      const char *rating;
      const char *requirement;
      bool compatible;
    };
    constexpr Catalog mods[]{
        {"mod-mycelium", "Mycelium Below", "Root & Branch",
         "Worlds · Biomes · Co-op", "18.8k", "96%", "Requires Base Content",
         true},
        {"mod-grapple", "Brassline Grapple Kit", "Clockwork Moss",
         "Mechanics · Traversal", "31.2k", "97%",
         "Requires Cartographer's Desk", true},
        {"mod-skybreak", "Skybreak Caverns", "Gale Assembly", "Worlds · Sky",
         "12.4k", "91%", "Requires game 1.4", true},
        {"mod-tide", "Abyssal Tide", "Bathysphere", "Worlds · Water", "17.2k",
         "93%", "Requires Underground Rivers", true},
        {"mod-depths", "Mirror Depths", "Parallax Union", "Quest · Challenge",
         "19.8k", "88%", "Requires Gubsy Mod API 0.2", false},
        {"mod-ember", "Ember Orchard", "Kindling Club", "Biome · Items", "8.1k",
         "94%", "Requires Base Content", true},
        {"mod-clock", "Clockwork Descent", "Ratchet House", "Quest · Machines",
         "27.7k", "98%", "Requires game 1.4", true},
        {"mod-fossil", "Fossil Gardens", "Marl & Fern", "Biome · Creatures",
         "11.9k", "90%", "Requires Mycelium Below", true},
        {"mod-moon", "Moonlit Markets", "Silver Cart", "Shops · Items", "42.3k",
         "95%", "Requires game 1.3", true},
        {"mod-coral", "Coral Reliquary", "Tideglass", "Quest · Water", "6.4k",
         "89%", "Requires API 0.3 preview", false},
        {"mod-iron", "Iron Root Arsenal", "Forge Choir", "Items · Combat",
         "23.6k", "92%", "Requires Base Content", true},
        {"mod-echo", "Echoing Markets", "Hollow Bell", "Audio · Shops", "14.0k",
         "96%", "Requires Moonlit Markets", true},
        {"mod-silk", "Silken Catacombs", "Threadbare", "Worlds · Traps", "9.7k",
         "87%", "Requires game 1.4", true},
        {"mod-ghost", "Friendly Ghosts", "Pale Picnic", "Creatures · Co-op",
         "38.5k", "99%", "Requires Base Content", true},
        {"mod-storm", "Temple Weather", "Northglass", "Effects · World",
         "21.1k", "94%", "Requires renderer 2.0", false},
        {"mod-camp", "Campfire Stories", "Wayfarer", "Quests · Dialogue",
         "5.8k", "92%", "Requires Old Lanterns", true},
        {"mod-vault", "The Violet Vault", "Archive Team", "Quest · Endgame",
         "29.2k", "97%", "Requires Violet Reach", true},
        {"mod-tiny", "Tiny Expeditions", "Pocket Moss", "Mode · Casual",
         "33.0k", "95%", "Requires game 1.4", true},
        {"mod-race", "Relay Races", "Split Second", "Mode · Co-op", "15.5k",
         "90%", "Requires networking 1.2", true},
        {"mod-legacy", "Legacy Room Pack", "Old Stone", "Rooms · Archive",
         "4.2k", "73%", "Built for game 0.9 only", false}};
    out << R"(<div class="catalog-tools"><input data-focus data-action="search-catalog" class="search-field" type="text" value=")"
        << escape_attribute(state_.mod_filter)
        << R"(" placeholder="Search the Gubsy mod catalog…"/><label class="check-control"><input data-focus data-action="compatible-value" class="native-toggle" type="checkbox" value="true")";
    if (state_.compatible_only)
      out << R"( checked="checked")";
    out << R"(/><span>Compatible only<small>Hide mods unusable on this build</small></span></label><button data-focus data-action="refresh-catalog" class="button">↻ Refresh</button></div><div class="master-detail"><section class="panel master-list scroll-list catalog-list"><div class="scroll-body"><small>)"
        << (state_.compatible_only ? "COMPATIBLE MODS" : "20 CATALOG MODS")
        << R"(</small>)";
    for (const Catalog &mod : mods) {
      if (state_.compatible_only && !mod.compatible)
        continue;
      if (!contains_ci(mod.name, state_.mod_filter) &&
          !contains_ci(mod.author, state_.mod_filter) &&
          !contains_ci(mod.category, state_.mod_filter) &&
          !contains_ci(mod.requirement, state_.mod_filter))
        continue;
      const size_t image_index = static_cast<size_t>(&mod - mods);
      out << R"(<button data-focus data-action="catalog-mod-)" << mod.name
          << R"(" class=")"
          << selected_class(state_.selected_mod == mod.name,
                            std::string("catalog-row ") +
                                (mod.compatible ? "" : "incompatible"))
          << R"("><img class="mod-thumb" src="../mods/)"
          << mod_image(image_index) << R"("/><span><strong>)" << mod.name
          << R"(</strong><small>)" << mod.category << R"( · by )" << mod.author
          << R"( · )" << mod.downloads << R"( downloads</small><em>)"
          << mod.requirement << R"(</em></span><b>★ )" << mod.rating
          << R"( positive</b><i></i></button>)";
    }
    out << R"(</div>)";
    out << R"(</section><aside class="panel detail mod-detail"><img class="hero-art" src="../mods/sheet-1-0.png"/><div class="tag-row"><span>WORLDS</span><span>BIOMES</span><span>CO-OP</span></div><small>CATALOG ENTRY</small><h2>)"
        << state_.selected_mod
        << R"(</h2><p>A hand-built content package with carefully integrated rooms, mechanics, artwork, and native co-op synchronization.</p><div class="impact"><h3>INSTALL PLAN</h3><div class="kv"><span>Selected</span><b>v1.1.0</b></div><div class="kv"><span>Download</span><b>84.2 MB</b></div><div class="kv"><span>Dependencies</span><b>2 packages</b></div><div class="kv"><span>Compatibility</span><b>Game 1.4 ready</b></div></div><div class="dependency"><h3>AUTOMATIC DEPENDENCIES</h3><div><span>↳ Base Content v1.4.0</span><b>INSTALLED</b></div><div><span>↳ Underground Rivers v2.2.0</span><b>WILL INSTALL</b></div></div><button data-focus data-action="install-add-session" class="button primary">Install &amp; add to current session</button><button data-focus data-action="install-only" class="button">Install only</button></aside></div>)";
  }
  return out.str();
}
