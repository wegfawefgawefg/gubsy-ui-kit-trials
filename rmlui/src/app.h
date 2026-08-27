#pragma once

#include "ui_state.h"

#include <RmlUi/Core/EventListener.h>
#include <SDL3/SDL_events.h>

#include <array>
#include <string>

namespace Rml {
class Context;
class Element;
class ElementDocument;
} // namespace Rml

class GubsyApp final : public Rml::EventListener {
public:
  explicit GubsyApp(Rml::Context *context);
  ~GubsyApp() override;

  bool Initialize(const std::string &document_path);
  void Update();
  void SetViewport(int width, int height);
  void SelectToolScreen(int index);
  void SetProviderState(int index);
  void SetGamepadStatus(int count, const std::string &name);
  bool HandleSdlEvent(const SDL_Event &event);
  bool RunSelfTest();
  void ProcessEvent(Rml::Event &event) override;

  Rml::ElementDocument *document() const { return document_; }
  const char *current_screen_name() const;

private:
  // mutable authored UI state
  UiState state_;

  // retained document lifecycle
  Rml::Context *context_ = nullptr;
  Rml::ElementDocument *document_ = nullptr;
  int viewport_width_ = 1280;
  int viewport_height_ = 720;
  bool dirty_ = true;
  std::string toast_;
  double toast_until_ = 0.0;

  // controller edit transaction
  int controller_x_latch_ = 0;
  int controller_y_latch_ = 0;
  bool controller_editing_ = false;
  std::string controller_edit_original_value_;

  // remembered local focus
  std::string pending_focus_action_;
  std::array<std::string, 6> content_focus_memory_;

  // document and interaction flow
  void MarkDirty();
  void Render();
  void RenderChrome();
  void HandleAction(const std::string &action);
  void NavigateFocus(int dx, int dy);
  void ActivateFocus();
  void FocusActiveNavigation();
  void FocusRememberedContent();
  void Back();
  void SetToast(std::string message);
  bool RunNavigationSelfTest();

  // routed screen markup
  std::string BuildCurrentScreen() const;
  std::string BuildPlayLobby() const;
  std::string BuildQuestPicker() const;
  std::string BuildRules() const;
  std::string BuildSessionMods() const;
  std::string BuildPlayers() const;
  std::string BuildSettings() const;
  std::string BuildControls() const;
  std::string BuildProgress() const;
  std::string BuildMods() const;
};
