#include "app.h"

std::string GubsyApp::BuildCurrentScreen() const {
  // build provider state surface
  if (state_.provider_state != "Populated") {
    if (state_.provider_state == "Loading")
      return R"(<div class="state-surface provider-state"><small>PROVIDER REQUEST IN FLIGHT</small><h2>Loading game data…</h2><p>The fixed shell and navigation remain stable while this provider resolves.</p><div class="loading-bar"><i></i></div></div>)";
    if (state_.provider_state == "Empty")
      return R"(<div class="state-surface provider-state"><small>VALID EMPTY STATE</small><h2>Nothing here yet</h2><p>This provider returned an empty collection. Create the first item or return to another section.</p><button data-focus data-action="create-first" class="button primary">Create first item</button></div>)";
    return R"(<div class="state-surface provider-state error-state"><small>PROVIDER ERROR</small><h2>Game data could not be loaded</h2><p>The native shell remains usable. Retry this provider without rebuilding or losing the rest of the menu state.</p><button data-focus data-action="retry-provider" class="button primary">Retry</button></div>)";
  }
  // route populated destination
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
