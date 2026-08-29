#include "view_builder.hpp"

namespace {

void panel(ViewBuilder& ui, std::string_view parent, const std::string& id,
           glayout::Length width = {glayout::LengthKind::Fill, 1.0f}) {
    ui.container(parent, id, glayout::ContainerKind::Column, width,
                 {glayout::LengthKind::Fill, 1.0f}, 6.0f, {12.0f, 10.0f, 12.0f, 10.0f});
}

void lobby(ViewBuilder& ui, const TrialModel& model, std::string_view content) {
    const std::string activity = model.text("activity", "Continue expedition");
    const bool continuing = activity == "Continue expedition";
    const bool solo = model.text("access", "Friends can join") == "Solo";
    ui.container(content, "play-workspace", ui.split(), {glayout::LengthKind::Fill, 1.0f},
                 {glayout::LengthKind::Fill, 1.0f}, 12.0f);
    panel(ui, "play-workspace", "setup");
    ui.label("setup", "quest-kicker", continuing ? "CONTINUE QUEST" : "START QUEST", 18.0f, 11.0f);
    ui.label("setup", "quest-title", "The Violet Reach", 30.0f, 22.0f);
    ui.label("setup", "quest-note", "The Glass Caverns · Latest checkpoint · Vega", 20.0f, 12.0f);
    ui.select("setup", "activity", "Activity", "activity",
              {"Continue expedition", "New expedition", "Arena run"}, "play-setup", 48.0f);
    ui.button("setup", "resume-point",
              continuing ? "Resume point\nLatest checkpoint · The Violet Reach"
                         : "Quest\nChoose a quest and starting route",
              "play:quest", "play-setup", 48.0f);
    ui.select("setup", "play-with", "Play with", "access",
              {"Solo", "Friends can join", "Invite only", "Public"}, "play-setup", 48.0f);
    ui.select("setup", "host-using", "Host using", "host",
              {"Automatic", "Host locally", "Dedicated relay"}, "play-setup", 48.0f);
    ui.button("setup", "expedition-rules", "Expedition rules\nStandard · 4 lives · ghost at 180s",
              "play:rules", "play-setup", 50.0f);
    ui.button("setup", "session-mods", "Session mods\n7 active · dependency set valid", "play:mods",
              "play-setup", 50.0f);
    ui.container("setup", "play-actions", glayout::ContainerKind::Row,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Pixels, 46.0f}, 8.0f);
    ui.button("play-actions", "pause-preview", "Pause preview", "toast:Preview paused",
              "play-actions", 46.0f);
    ui.button("play-actions", "begin-session",
              continuing ? "▶ Resume latest checkpoint" : "▶ Begin new expedition", "start-session",
              "play-actions", 46.0f);

    panel(ui, "play-workspace", "party", {glayout::LengthKind::Pixels, 330.0f});
    ui.label("party", "party-title", "PLAYERS\nYour party", 54.0f, 18.0f);
    ui.button("party", "player-one", "P1   Moss\n      Xbox Wireless Controller        READY",
              "players", "play-party", 62.0f);
    if (!solo) {
        for (int slot = 2; slot <= 4; ++slot)
            ui.button("party", "open-slot-" + std::to_string(slot),
                      "+   Open slot\n      Invite a friend or add locally",
                      "toast:Player slot opened", "play-party", 56.0f);
        ui.button("party", "invite", "Invite / copy link", "toast:Invite copied", "play-party");
        ui.button("party", "find-games", "Friends & public games", "toast:Browser opened",
                  "play-party");
    }
    ui.label("party", "summary",
             "CONTENT                 7 mods\nRULESET                 "
             "Standard\nNETWORK                 Automatic",
             80.0f, 12.0f);
    ui.focus_group("play-setup", "activity", "nav-Play");
    ui.focus_group("play-actions", "pause-preview", "nav-Play");
    ui.focus_group("play-party", "player-one", "nav-Play");
    ui.edge("nav-Play", gview::NavAction::Right, "activity");
    ui.group_edge("play-setup", gview::NavAction::Left, "rail");
    ui.group_edge("play-setup", gview::NavAction::Right, "play-party");
    ui.group_edge("play-party", gview::NavAction::Left, "play-setup");
    ui.group_edge("play-setup", gview::NavAction::Down, "play-actions");
    ui.group_edge("play-actions", gview::NavAction::Up, "play-setup");
}

void quest(ViewBuilder& ui, const TrialModel&, std::string_view content) {
    ui.button(content, "quest-back", "‹ Back to lobby", "play:lobby", "quest", 44.0f);
    ui.container(content, "quest-workspace", ui.split(), {glayout::LengthKind::Fill, 1.0f},
                 {glayout::LengthKind::Fill, 1.0f}, 12.0f);
    panel(ui, "quest-workspace", "quest-list", {glayout::LengthKind::Pixels, 390.0f});
    ui.scrolling("quest-list");
    ui.label("quest-list", "quest-list-title", "EXPEDITIONS AND QUESTS", 28.0f, 11.0f);
    constexpr const char* quests[]{"The Violet Reach", "The Sunken Road", "A Green Beginning",
                                   "The Glass Pilgrim"};
    for (const char* name : quests)
        ui.button("quest-list", std::string("quest-") + name,
                  name + std::string("\nQuest route · 4–7 stages"), std::string("select:") + name,
                  "quest", 70.0f);
    panel(ui, "quest-workspace", "quest-detail");
    ui.scrolling("quest-detail");
    ui.label("quest-detail", "quest-detail-kicker", "TEMPLE FRONTIER", 24.0f, 11.0f);
    ui.label("quest-detail", "quest-detail-title", "The Violet Reach", 42.0f, 26.0f);
    ui.label("quest-detail", "quest-description",
             "Follow a fractured relay signal through flooded archives, fungal "
             "crossings, and a temple complex waking beneath the mountain.",
             78.0f, 14.0f);
    ui.image("quest-detail", "quest-art", "splonks-title", 132.0f);
    ui.label("quest-detail", "quest-route",
             "QUEST ROUTE\n1 North Pass   2 Mushroom Crossing   3 Flooded "
             "Archive\n4 Temple Gate   5 The Relay   6 Violet Core",
             100.0f, 14.0f);
    ui.button("quest-detail", "use-quest", "Use this checkpoint", "play:lobby", "quest", 52.0f);
    ui.focus_group("quest", "quest-back", "nav-Play");
    ui.edge("nav-Play", gview::NavAction::Right, "quest-back");
}

void rules(ViewBuilder& ui, const TrialModel&, std::string_view content) {
    ui.container(content, "rules-header", glayout::ContainerKind::Row,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Pixels, 52.0f}, 8.0f);
    ui.button("rules-header", "rules-back", "‹ Back to lobby", "play:lobby", "rules", 52.0f);
    ui.label("rules-header", "rules-title", "EXPEDITION SETTINGS · 10 rules · 1 from mods", 52.0f,
             16.0f);
    ui.button("rules-header", "rules-reset", "Reset defaults", "toast:Rules reset", "rules", 52.0f);
    ui.container(content, "rules-workspace", ui.split(), {glayout::LengthKind::Fill, 1.0f},
                 {glayout::LengthKind::Fill, 1.0f}, 12.0f);
    panel(ui, "rules-workspace", "rules-list");
    ui.layout("rules-list").clip = true;
    ui.spec("rules-list").control = gview::ControlKind::ScrollArea;
    ui.select("rules-list", "difficulty", "Expedition difficulty", "difficulty",
              {"Relaxed", "Standard", "Dangerous", "Nightmare"}, "rules");
    ui.select("rules-list", "variation", "Stage variation", "variation",
              {"Quest-authored", "Remixed", "Fully random"}, "rules");
    ui.slider("rules-list", "shared-lives", "Shared lives", "shared-lives", "rules", 1, 9, 1);
    ui.slider("rules-list", "starting-health", "Starting health", "starting-health", "rules", 1, 9,
              1);
    ui.slider("rules-list", "ghost-arrival", "Ghost arrival", "ghost-arrival", "rules", 30, 300,
              10);
    ui.select("rules-list", "shop-frequency", "Shop frequency", "shop-frequency",
              {"Rare", "Normal", "Frequent"}, "rules");
    ui.toggle("rules-list", "shortcuts", "Discovered shortcuts", "shortcuts", "rules");
    ui.toggle("rules-list", "treasury", "Shared treasury", "treasury", "rules");
    ui.toggle("rules-list", "friendly-fire", "Friendly fire", "friendly-fire", "rules");
    ui.select("rules-list", "lantern-fuel", "Lantern fuel · MOD: Old Lanterns", "lantern-fuel",
              {"Scarce", "Standard", "Generous"}, "rules");
    ui.layout("rules-list").clip = true;
    ui.layout("rules-list").container = glayout::ContainerKind::Column;
    ui.layout("rules-list").children.front().clip = false;
    panel(ui, "rules-workspace", "rule-detail", {glayout::LengthKind::Pixels, 420.0f});
    ui.scrolling("rule-detail");
    ui.label("rule-detail", "rule-kicker", "SELECTED RULE", 26.0f, 11.0f);
    ui.label("rule-detail", "rule-name", "Discovered shortcuts", 52.0f, 28.0f);
    ui.label("rule-detail", "rule-description",
             "Lets the party enter from previously discovered quest junctions. "
             "Changes serialize into the session manifest and synchronize before "
             "launch.",
             100.0f, 14.0f);
    ui.label("rule-detail", "rule-context",
             "SESSION EFFECT\nActivity      Continue expedition\nAuthority     "
             "Lobby host\nSynced        Before launch",
             120.0f, 13.0f);
    ui.focus_group("rules", "rules-back", "nav-Play");
    ui.edge("nav-Play", gview::NavAction::Right, "rules-back");
}

void session_mods(ViewBuilder& ui, const TrialModel&, std::string_view content) {
    ui.button(content, "mods-back", "‹ Back to lobby", "play:lobby", "session-mods", 44.0f);
    ui.container(content, "session-mod-workspace", ui.split(), {glayout::LengthKind::Fill, 1.0f},
                 {glayout::LengthKind::Fill, 1.0f}, 12.0f);
    panel(ui, "session-mod-workspace", "session-mod-list");
    ui.scrolling("session-mod-list");
    ui.label("session-mod-list", "session-mod-title", "CURRENT SESSION SET · 7 PACKAGES", 34.0f,
             12.0f);
    constexpr const char* mods[]{"Base Content",       "Cartographer's Desk", "Old Lanterns",
                                 "Underground Rivers", "Mycelium Below",      "Temple Weather",
                                 "Pocket Expedition"};
    for (const char* mod : mods)
        ui.button("session-mod-list", std::string("session-") + mod,
                  mod + std::string("\nActive · dependencies satisfied"),
                  std::string("select:") + mod, "session-mods", 56.0f);
    panel(ui, "session-mod-workspace", "session-mod-detail", {glayout::LengthKind::Pixels, 430.0f});
    ui.scrolling("session-mod-detail");
    ui.image("session-mod-detail", "session-mod-art", "mod-3", 140.0f);
    ui.label("session-mod-detail", "session-mod-name", "Old Lanterns", 42.0f, 26.0f);
    ui.label("session-mod-detail", "session-mod-info",
             "Update available. Required by Temple Weather and Pocket "
             "Expedition. Exact versions are recorded with checkpoints.",
             86.0f, 14.0f);
    ui.button("session-mod-detail", "browse-add", "Browse & add mods", "mods:browse",
              "session-mods");
    ui.button("session-mod-detail", "update-session-mod", "Update in this set",
              "toast:Update planned", "session-mods");
    ui.button("session-mod-detail", "remove-session-mod", "Remove with dependency plan",
              "toast:Dependency plan opened", "session-mods");
    ui.focus_group("session-mods", "mods-back", "nav-Play");
    ui.edge("nav-Play", gview::NavAction::Right, "mods-back");
}

} // namespace

void build_play(ViewBuilder& ui, const TrialModel& model, std::string_view content) {
    switch (model.play_page) {
    case PlayPage::Lobby:
        lobby(ui, model, content);
        break;
    case PlayPage::Quest:
        quest(ui, model, content);
        break;
    case PlayPage::Rules:
        rules(ui, model, content);
        break;
    case PlayPage::SessionMods:
        session_mods(ui, model, content);
        break;
    }
}
