#include "view_builder.hpp"

namespace {

void tabs(ViewBuilder& ui, std::string_view content, const TrialModel& model) {
    ui.container(content, "control-tabs", glayout::ContainerKind::Row,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Pixels, 44.0f}, 4.0f);
    for (const char* tab : {"Bindings", "Devices", "Input tuning"})
        ui.button("control-tabs", std::string("control-tab-") + tab, tab,
                  std::string("controls-tab:") + tab, "control-tabs", 44.0f,
                  gview::ActivationPolicy::OnFocus);
    ui.spec(std::string("control-tab-") + model.controls_tab).selected = true;
    ui.focus_group("control-tabs", std::string("control-tab-") + model.controls_tab,
                   "nav-Controls");
}

void bindings(ViewBuilder& ui, std::string_view content) {
    ui.container(content, "binding-toolbar", glayout::ContainerKind::Row,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Pixels, 46.0f}, 8.0f);
    ui.button("binding-toolbar", "binding-filter", "Filter actions…", "toast:Search focused",
              "controls-content", 46.0f);
    ui.select("binding-toolbar", "binding-profile", "", "binding-profile",
              {"Default Binds", "Arcade Binds", "Accessibility"}, "controls-content", 46.0f);
    for (const char* action : {"New", "Rename", "Reset", "Delete"})
        ui.button("binding-toolbar", std::string("binding-") + action, action,
                  std::string("toast:") + action + " binding profile", "controls-content", 46.0f);
    ui.container(content, "binding-workspace", ui.split(), {glayout::LengthKind::Fill, 1.0f},
                 {glayout::LengthKind::Fill, 1.0f}, 12.0f);
    ui.container("binding-workspace", "action-list", glayout::ContainerKind::Column,
                 {glayout::LengthKind::Pixels, 430.0f}, {glayout::LengthKind::Fill, 1.0f}, 5.0f,
                 {12.0f, 10.0f, 12.0f, 10.0f});
    ui.layout("action-list").clip = true;
    ui.spec("action-list").control = gview::ControlKind::ScrollArea;
    constexpr const char* actions[]{"Menu Up",  "Menu Down", "Menu Left", "Menu Right",
                                    "Activate", "Cancel",    "Move",      "Look",
                                    "Jump",     "Attack",    "Use",       "Pause"};
    for (const char* action : actions)
        ui.button("action-list", std::string("action-") + action,
                  action + std::string("\nMultiple keyboard and device bindings"),
                  std::string("select:") + action, "controls-content", 54.0f);
    ui.container("binding-workspace", "binding-detail", glayout::ContainerKind::Column,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Fill, 1.0f}, 7.0f,
                 {14.0f, 12.0f, 14.0f, 12.0f});
    ui.scrolling("binding-detail");
    ui.label("binding-detail", "binding-title", "SELECTED ACTION\nMenu Up", 62.0f, 25.0f);
    ui.button("binding-detail", "bind-one", "1   D-Pad Up                     Replace   ×",
              "binding:replace", "controls-content", 48.0f);
    ui.button("binding-detail", "bind-two", "2   Keyboard W                  Replace   ×",
              "binding:replace", "controls-content", 48.0f);
    ui.button("binding-detail", "bind-three", "3   Keyboard ↑                  Replace   ×",
              "binding:replace", "controls-content", 48.0f);
    ui.button("binding-detail", "add-binding", "+ Add binding", "binding:add", "controls-content");
    ui.label("binding-detail", "binding-methods",
             "ADD BINDING METHODS\nListen for next input · Perform an input · "
             "Browse device controls\nManual source: Right Trigger → Axis to "
             "Button · threshold 0.62 rising",
             110.0f, 13.0f);
    ui.button("binding-detail", "listen-input", "Listen for input", "binding:listen",
              "controls-content");
    ui.button("binding-detail", "browse-controls", "Browse device controls", "binding:browse",
              "controls-content");
}

void devices(ViewBuilder& ui, std::string_view content) {
    ui.container(content, "control-device-workspace", ui.split(), {glayout::LengthKind::Fill, 1.0f},
                 {glayout::LengthKind::Fill, 1.0f}, 12.0f);
    ui.container("control-device-workspace", "control-device-list", glayout::ContainerKind::Column,
                 {glayout::LengthKind::Pixels, 440.0f}, {glayout::LengthKind::Fill, 1.0f}, 6.0f,
                 {12.0f, 10.0f, 12.0f, 10.0f});
    ui.scrolling("control-device-list");
    constexpr const char* devices[]{"Keyboard + Mouse",
                                    "Xbox Wireless Controller",
                                    "DualSense Wireless Controller",
                                    "T.16000M Flight Stick",
                                    "T-LCM Pedals",
                                    "Stream Deck Macro Pad"};
    for (const char* device : devices)
        ui.button("control-device-list", std::string("input-device-") + device,
                  device + std::string("\nConnected · owner and raw input available"),
                  std::string("select:") + device, "controls-content", 62.0f);
    ui.container("control-device-workspace", "control-device-detail",
                 glayout::ContainerKind::Column, {glayout::LengthKind::Fill, 1.0f},
                 {glayout::LengthKind::Fill, 1.0f}, 8.0f, {14.0f, 12.0f, 14.0f, 12.0f});
    ui.scrolling("control-device-detail");
    ui.label("control-device-detail", "input-device-title",
             "DEVICE OWNERSHIP\nXbox Wireless Controller", 62.0f, 24.0f);
    ui.button("control-device-detail", "owner-unassigned", "○ Unassigned", "toast:Unassigned",
              "controls-content");
    ui.button("control-device-detail", "owner-moss", "P1 Moss · ACTIVE", "toast:Assigned to Moss",
              "controls-content");
    ui.button("control-device-detail", "owner-vega", "P2 Vega", "toast:Assigned to Vega",
              "controls-content");
    ui.label("control-device-detail", "raw-device-input",
             "LIVE INPUT EXPLORER\nAxis 4 Right Trigger     0.742\nButton 1 "
             "South           UP\nHat 0 D-Pad              UP-RIGHT",
             146.0f, 14.0f);
    ui.button("control-device-detail", "device-identify", "Identify", "toast:Device identified",
              "controls-content");
}

void tuning(ViewBuilder& ui, std::string_view content) {
    ui.container(content, "tuning-workspace", ui.split(), {glayout::LengthKind::Fill, 1.0f},
                 {glayout::LengthKind::Fill, 1.0f}, 12.0f);
    ui.container("tuning-workspace", "tuning-list", glayout::ContainerKind::Column,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Fill, 1.0f}, 7.0f,
                 {12.0f, 10.0f, 12.0f, 10.0f});
    ui.scrolling("tuning-list");
    ui.slider("tuning-list", "look-sensitivity", "Look sensitivity", "look-sensitivity",
              "controls-content", 0, 1, 0.01);
    ui.slider("tuning-list", "stick-deadzone", "Stick deadzone", "stick-deadzone",
              "controls-content", 0, 1, 0.01);
    ui.slider("tuning-list", "vibration", "Vibration strength", "vibration", "controls-content", 0,
              1, 0.01);
    ui.slider("tuning-list", "trigger-deadzone", "Trigger deadzone", "trigger-deadzone",
              "controls-content", 0, 1, 0.01);
    ui.select("tuning-list", "response-curve", "Response curve", "response-curve",
              {"Linear", "Smooth", "Aggressive", "Custom"}, "controls-content");
    ui.toggle("tuning-list", "invert-y", "Invert Y-axis", "invert-y", "controls-content");
    ui.button("tuning-list", "reset-tuning", "Reset input profile", "toast:Input profile reset",
              "controls-content");
    ui.container("tuning-workspace", "response-detail", glayout::ContainerKind::Column,
                 {glayout::LengthKind::Pixels, 430.0f}, {glayout::LengthKind::Fill, 1.0f}, 8.0f,
                 {14.0f, 12.0f, 14.0f, 12.0f});
    ui.scrolling("response-detail");
    ui.label("response-detail", "response-title", "DEVICE RESPONSE\nLive response", 64.0f, 25.0f);
    ui.label("response-detail", "response-graph",
             "              ●\n\nRAW INPUT      0.742\nQUALIFIED      0.611", 200.0f, 17.0f,
             gview::TextAlign::Center);
    ui.label("response-detail", "response-summary",
             "Device       Xbox Wireless Controller\nDeadzone     12%\nVibration "
             "   80%\nCurve        Smooth",
             120.0f, 14.0f);
}

} // namespace

void build_controls(ViewBuilder& ui, const TrialModel& model, std::string_view content) {
    tabs(ui, content, model);
    const std::string owner = std::string("control-tab-") + model.controls_tab;
    if (model.controls_tab == "Devices") {
        devices(ui, content);
        ui.focus_scope("control-device-list", "control-device-list");
        ui.focus_scope("control-device-detail", "control-device-detail");
        ui.focus_group("control-device-list", {}, owner);
        ui.focus_group("control-device-detail", "owner-unassigned", owner);
        ui.group_edge("control-tabs", gview::NavAction::Down, "control-device-list");
        ui.group_edge("control-device-list", gview::NavAction::Up, "control-tabs");
        ui.group_edge("control-device-list", gview::NavAction::Right,
                      "control-device-detail");
        ui.group_edge("control-device-detail", gview::NavAction::Left,
                      "control-device-list");
        ui.group_edge("control-device-detail", gview::NavAction::Up, "control-tabs");
    } else if (model.controls_tab == "Input tuning") {
        tuning(ui, content);
        ui.focus_scope("tuning-list", "control-tuning");
        ui.focus_group("control-tuning", "look-sensitivity", owner);
        ui.group_edge("control-tabs", gview::NavAction::Down, "control-tuning");
        ui.group_edge("control-tuning", gview::NavAction::Up, "control-tabs");
    } else {
        bindings(ui, content);
        ui.focus_scope("binding-toolbar", "binding-toolbar");
        ui.focus_scope("action-list", "binding-actions");
        ui.focus_scope("binding-detail", "binding-detail");
        ui.focus_group("binding-toolbar", "binding-filter", owner);
        ui.focus_group("binding-actions", "action-Menu Up", owner);
        ui.focus_group("binding-detail", "bind-one", owner);
        ui.group_edge("control-tabs", gview::NavAction::Down, "binding-toolbar");
        ui.group_edge("binding-toolbar", gview::NavAction::Up, "control-tabs");
        ui.group_edge("binding-toolbar", gview::NavAction::Down, "binding-actions");
        ui.group_edge("binding-actions", gview::NavAction::Up, "binding-toolbar");
        ui.group_edge("binding-actions", gview::NavAction::Right, "binding-detail");
        ui.group_edge("binding-detail", gview::NavAction::Left, "binding-actions");
        ui.group_edge("binding-detail", gview::NavAction::Up, "control-tabs");
    }
    ui.edge("nav-Controls", gview::NavAction::Right,
            std::string("control-tab-") + model.controls_tab);
    ui.group_edge("control-tabs", gview::NavAction::Left, "rail");
}
