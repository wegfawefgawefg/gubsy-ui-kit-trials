#include "app.h"
#include "ui_markup.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/SystemInterface.h>

// Document lifetime and externally selected demo state.

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
    state_.player_tab = "Local players";
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
      status->SetInnerRML("● CONTROLLER · " + markup::escape_attribute(name));
    else
      status->SetInnerRML("○ CONNECT CONTROLLER");
  }
}
