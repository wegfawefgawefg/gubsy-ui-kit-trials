#include "app.hpp"

#include <cstdio>

// What is: Short-lived game feedback that never becomes persistent screen
// content.
void TrialApp::show_toast(std::string message) {
    model_.toast = std::move(message);
    toast_expires_at_ = SDL_GetTicks() + 2500;
    ++model_.revision;
    model_.rebuild = true;
}

// What is: One clearing path for timeout, dismissal, and route changes.
void TrialApp::clear_toast() {
    if (model_.toast.empty()) return;
    model_.toast.clear();
    toast_expires_at_ = 0;
    ++model_.revision;
    model_.rebuild = true;
}

// What is: Trial-owned route, modal, selection, and notification actions.
void TrialApp::action(std::string_view action_name, gview::NodeIndex) {
    const std::string action(action_name);
    const auto suffix = [&](std::string_view prefix) { return action.substr(prefix.size()); };
    if (action == "quit") running_ = false;
    else if (action == "modal:cancel") {
        model_.modal.clear();
        model_.rebuild = true;
    } else if (action == "modal:confirm") {
        model_.modal.clear();
        show_toast("Destructive change applied");
    } else if (action == "modal:delete" || action == "modal:uninstall") {
        model_.modal = action == "modal:delete" ? "Delete campaign?"
                                                 : "Uninstall dependent packages?";
        model_.pending_focus = "modal-cancel";
        model_.rebuild = true;
    } else if (action.rfind("destination:", 0) == 0) {
        clear_toast();
        const std::string name = suffix("destination:");
        if (name == "Play") model_.destination = Destination::Play;
        else if (name == "Players") model_.destination = Destination::Players;
        else if (name == "Settings") model_.destination = Destination::Settings;
        else if (name == "Controls") model_.destination = Destination::Controls;
        else if (name == "Progress") model_.destination = Destination::Progress;
        else if (name == "Mods") model_.destination = Destination::Mods;
        model_.rebuild = true;
    } else if (action == "players") {
        clear_toast();
        model_.destination = Destination::Players;
        model_.players_tab = "Local players";
        model_.rebuild = true;
    } else if (action.rfind("players-tab:", 0) == 0) {
        model_.players_tab = suffix("players-tab:");
        model_.rebuild = true;
    } else if (action.rfind("settings-tab:", 0) == 0) {
        model_.settings_tab = suffix("settings-tab:");
        model_.rebuild = true;
    } else if (action.rfind("controls-tab:", 0) == 0) {
        model_.controls_tab = suffix("controls-tab:");
        model_.rebuild = true;
    } else if (action.rfind("mods-tab:", 0) == 0) {
        model_.mods_tab = suffix("mods-tab:");
        model_.selected_mod =
            model_.mods_tab == "Browse catalog" ? "Mycelium Below" : "Old Lanterns";
        model_.rebuild = true;
    } else if (action == "play:lobby") {
        model_.destination = Destination::Play;
        model_.play_page = PlayPage::Lobby;
        model_.rebuild = true;
    } else if (action == "play:quest" || action == "play:rules" || action == "play:mods") {
        if (action == "play:quest") model_.play_page = PlayPage::Quest;
        else if (action == "play:rules") model_.play_page = PlayPage::Rules;
        else model_.play_page = PlayPage::SessionMods;
        model_.rebuild = true;
    } else if (action == "mods:browse") {
        model_.destination = Destination::Mods;
        model_.mods_tab = "Browse catalog";
        model_.rebuild = true;
    } else if (action == "controls:devices") {
        model_.destination = Destination::Controls;
        model_.controls_tab = "Devices";
        model_.rebuild = true;
    } else if (action.rfind("session-mod-select:", 0) == 0) {
        model_.selected_session_mod = suffix("session-mod-select:");
        ++model_.revision;
        model_.rebuild = true;
    } else if (action.rfind("mod-select:", 0) == 0) {
        model_.selected_mod = suffix("mod-select:");
        ++model_.revision;
        model_.rebuild = true;
    } else if (action.rfind("select:", 0) == 0) {
        model_.selected = suffix("select:");
        ++model_.revision;
    } else if (action == "toast:clear") clear_toast();
    else if (action.rfind("toast:", 0) == 0) show_toast(suffix("toast:"));
    else if (action == "back" && !model_.modal.empty()) {
        model_.modal.clear();
        model_.rebuild = true;
    } else if (action == "back" && model_.destination == Destination::Play &&
               model_.play_page != PlayPage::Lobby) {
        model_.play_page = PlayPage::Lobby;
        model_.rebuild = true;
    } else if (action == "start-session") show_toast("Session launch requested");
    else if (!action.empty()) std::fprintf(stderr, "unhandled trial action: %s\n", action.c_str());
}
