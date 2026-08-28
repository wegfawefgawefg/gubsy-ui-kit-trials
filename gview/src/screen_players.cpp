#include "view_builder.hpp"

namespace {

void tabs(ViewBuilder& ui, std::string_view content, const TrialModel& model) {
    ui.container(content, "player-tabs", glayout::ContainerKind::Row,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Pixels, 44.0f}, 4.0f);
    for (const char* tab : {"Local players", "Profiles", "Devices"})
        ui.button("player-tabs", std::string("player-tab-") + tab, tab,
                  std::string("players-tab:") + tab, "player-tabs", 44.0f,
                  gview::ActivationPolicy::OnFocus);
    ui.spec(std::string("player-tab-") + model.players_tab).selected = true;
}

void local_players(ViewBuilder& ui, std::string_view content) {
    ui.container(content, "players-workspace", ui.split(), {glayout::LengthKind::Fill, 1.0f},
                 {glayout::LengthKind::Fill, 1.0f}, 12.0f);
    ui.container("players-workspace", "roster", glayout::ContainerKind::Column,
                 {glayout::LengthKind::Pixels, 420.0f}, {glayout::LengthKind::Fill, 1.0f}, 7.0f,
                 {12.0f, 10.0f, 12.0f, 10.0f});
    ui.label("roster", "roster-title", "LOCAL ROSTER · 2 / 4 players", 34.0f, 12.0f);
    ui.button("roster", "roster-moss", "P1  Moss\nDefault Binds · Xbox controller", "select:Moss",
              "players-content", 68.0f);
    ui.button("roster", "roster-vega", "P2  Vega\nArcade Binds · Keyboard + mouse", "select:Vega",
              "players-content", 68.0f);
    ui.button("roster", "add-local-player", "+ Add local player", "toast:Player added",
              "players-content", 52.0f);
    ui.container("players-workspace", "player-detail", glayout::ContainerKind::Column,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Fill, 1.0f}, 8.0f,
                 {14.0f, 12.0f, 14.0f, 12.0f});
    ui.scrolling("player-detail");
    ui.label("player-detail", "local-kicker", "LOCAL PLAYER", 24.0f, 11.0f);
    ui.label("player-detail", "local-name", "P1 · Moss", 44.0f, 27.0f);
    ui.select("player-detail", "player-profile", "Profile", "player-profile",
              {"Moss", "Vega", "Guest"}, "players-content");
    ui.select("player-detail", "player-binds", "Binding profile", "player-binds",
              {"Default Binds", "Arcade Binds", "Accessibility"}, "players-content");
    ui.select("player-detail", "player-tuning", "Input profile", "player-tuning",
              {"Standard", "Precise", "Vehicle"}, "players-content");
    ui.label("player-detail", "assigned-devices",
             "ASSIGNED DEVICES\nXbox Wireless Controller\nKeyboard + "
             "Mouse\nT-LCM Pedals",
             120.0f, 14.0f);
    ui.button("player-detail", "assign-device", "Assign another device", "controls:devices",
              "players-content");
    ui.toggle("player-detail", "ready", "Ready for session", "player-ready", "players-content");
}

void profiles(ViewBuilder& ui, std::string_view content) {
    ui.container(content, "profiles", glayout::ContainerKind::Column,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Fill, 1.0f}, 10.0f,
                 {14.0f, 12.0f, 14.0f, 12.0f});
    ui.scrolling("profiles");
    ui.container("profiles", "profile-cards", glayout::ContainerKind::Row,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Pixels, 134.0f}, 12.0f);
    ui.button("profile-cards", "profile-moss", "MO\nMoss\n38h 22m · 84 runs · 21 wins",
              "select:Moss", "players-content", 134.0f);
    ui.button("profile-cards", "profile-vega", "VE\nVega\n14h 11m · 31 runs · 4 wins",
              "select:Vega", "players-content", 134.0f);
    ui.button("profile-cards", "profile-guest", "GU\nGuest\nNever · 0 runs · 0 wins",
              "select:Guest", "players-content", 134.0f);
    ui.button("profile-cards", "new-profile", "+ New profile", "toast:Profile editor opened",
              "players-content", 134.0f);
    ui.label("profiles", "profile-data-title", "PROFILE DATA IS NOT A SAVE\nMoss's history", 58.0f,
             22.0f);
    ui.label("profiles", "profile-statistics",
             "84 PLAYS        21 WINS        138 DEATHS        2.4M SCORE        "
             "42 REPLAYS",
             82.0f, 22.0f, gview::TextAlign::Center);
    ui.label("profiles", "profile-explanation",
             "Achievements, aggregate statistics, unlocks, scores, and replays "
             "survive across campaigns and checkpoints.",
             64.0f, 14.0f);
    ui.button("profiles", "profile-replays", "Browse replays", "toast:Replay browser opened",
              "players-content");
}

void devices(ViewBuilder& ui, std::string_view content) {
    ui.container(content, "devices-workspace", ui.split(), {glayout::LengthKind::Fill, 1.0f},
                 {glayout::LengthKind::Fill, 1.0f}, 12.0f);
    ui.container("devices-workspace", "device-list", glayout::ContainerKind::Column,
                 {glayout::LengthKind::Pixels, 440.0f}, {glayout::LengthKind::Fill, 1.0f}, 6.0f,
                 {12.0f, 10.0f, 12.0f, 10.0f});
    ui.scrolling("device-list");
    constexpr const char* devices[]{"Keyboard + Mouse", "Xbox Wireless Controller",
                                    "DualSense Wireless Controller", "T.16000M Flight Stick",
                                    "T-LCM Pedals"};
    for (const char* device : devices)
        ui.button("device-list", std::string("device-") + device,
                  device + std::string("\nConnected · input explorer ready"),
                  std::string("select:") + device, "players-content", 64.0f);
    ui.container("devices-workspace", "device-detail", glayout::ContainerKind::Column,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Fill, 1.0f}, 8.0f,
                 {14.0f, 12.0f, 14.0f, 12.0f});
    ui.scrolling("device-detail");
    ui.label("device-detail", "device-title", "RECOGNIZED INPUT DEVICE\nXbox Wireless Controller",
             62.0f, 22.0f);
    ui.label("device-detail", "device-note",
             "Assign one device to a local player. A player can combine "
             "gamepads, wheels, pedals, sticks, and macro pads.",
             72.0f, 14.0f);
    ui.button("device-detail", "device-unassigned", "○  Unassigned", "toast:Device unassigned",
              "players-content");
    ui.button("device-detail", "device-moss", "P1  Moss · Default Binds", "toast:Assigned to Moss",
              "players-content");
    ui.button("device-detail", "device-vega", "P2  Vega · Arcade Binds", "toast:Assigned to Vega",
              "players-content");
    ui.label("device-detail", "input-explorer",
             "LIVE INPUT EXPLORER\nAxis 4 · Right Trigger       0.742\nButton 1 "
             "· South             UP\nHat 0 · D-Pad                UP-RIGHT",
             138.0f, 14.0f);
    ui.button("device-detail", "identify-device", "Identify device", "toast:Device rumbled",
              "players-content");
}

} // namespace

void build_players(ViewBuilder& ui, const TrialModel& model, std::string_view content) {
    tabs(ui, content, model);
    if (model.players_tab == "Profiles") profiles(ui, content);
    else if (model.players_tab == "Devices") devices(ui, content);
    else local_players(ui, content);
    const std::string entry = model.players_tab == "Profiles"  ? "profile-moss"
                              : model.players_tab == "Devices" ? "device-Keyboard + Mouse"
                                                               : "roster-moss";
    ui.focus_group("player-tabs", std::string("player-tab-") + model.players_tab, "nav-Players");
    ui.focus_group("players-content", entry, std::string("player-tab-") + model.players_tab);
    ui.edge("nav-Players", gview::NavAction::Right, std::string("player-tab-") + model.players_tab);
    ui.edge(std::string("player-tab-") + model.players_tab, gview::NavAction::Down, entry);
}
