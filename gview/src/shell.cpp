#include "view_builder.hpp"

namespace {

std::string active_nav(const TrialModel& model) {
    return "nav-" + destination_name(model.destination);
}

} // namespace

// Composes fixed game chrome around independently authored destination content.
gview::View build_shell_view(const TrialModel& model, int width, int height) {
    ViewBuilder ui("gubsy-shell", width, height);
    const float s = ui.scale();
    ui.container("root", "shell-frame", glayout::ContainerKind::Column,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Fill, 1.0f});
    ui.container("shell-frame", "header", glayout::ContainerKind::Row,
                 {glayout::LengthKind::Fill, 1.0f},
                 {glayout::LengthKind::Pixels, ui.compact() ? 46.0f : 56.0f}, 12.0f,
                 {16.0f, 6.0f, 16.0f, 6.0f});
    ui.label("header", "brand", "G  G U B S Y  S H E L L", 44.0f, 14.0f);
    ui.label("header", "connection", "●  OFFLINE    |    VEGA", 44.0f, 13.0f,
             gview::TextAlign::Center);
    if (!ui.compact())
        ui.label("header", "viewport",
                 "DEMO · " + model.provider_state + "    " + std::to_string(width) + " × " +
                     std::to_string(height),
                 44.0f, 13.0f, gview::TextAlign::End);

    ui.container("shell-frame", "body", glayout::ContainerKind::Row,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Fill, 1.0f}, 0.0f);
    struct Nav {
        const char* name;
        const char* note;
    };
    constexpr Nav entries[]{
        {"Play", "Continue or start"},           {"Players", "Profiles & devices"},
        {"Settings", "Game preferences"},        {"Controls", "Bindings & input"},
        {"Progress", "Campaigns & checkpoints"}, {"Mods", "Installed content"}};
    if (!ui.compact()) {
        ui.container("body", "nav", glayout::ContainerKind::Column,
                     {glayout::LengthKind::Pixels, 244.0f}, {glayout::LengthKind::Fill, 1.0f}, 8.0f,
                     {16.0f, 16.0f, 16.0f, 16.0f});
        ui.label("nav", "profile", "VE   ACTIVE PROFILE\n       Vega", 72.0f, 13.0f);
        for (const Nav& entry : entries) {
            const std::string id = std::string("nav-") + entry.name;
            ui.button("nav", id, std::string("›  ") + entry.name + "\n    " + entry.note,
                      std::string("destination:") + entry.name, "rail", 52.0f,
                      gview::ActivationPolicy::OnFocus);
            ui.spec(id).text_style.size = 14.0f * s;
            ui.spec(id).selected = id == active_nav(model);
            if (id == active_nav(model)) {
                ui.layout(id).padding.left = 18.0f * s;
                ui.layout(id).padding.right = 10.0f * s;
            }
        }
        ui.container("nav", "nav-spacer", glayout::ContainerKind::Absolute,
                     {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Fill, 1.0f});
        ui.button("nav", "nav-Quit", "×  Quit\n    Return to desktop", "quit", "rail", 58.0f);
        ui.spec("nav-Quit").text_style.size = 14.0f * s;
    }

    ui.container("body", "main", glayout::ContainerKind::Column, {glayout::LengthKind::Fill, 1.0f},
                 {glayout::LengthKind::Fill, 1.0f}, 10.0f,
                 {ui.compact() ? 10.0f : 32.0f, ui.compact() ? 6.0f : 18.0f,
                  ui.compact() ? 10.0f : 32.0f, ui.compact() ? 4.0f : 12.0f});
    ui.label("main", "breadcrumb", "SPLONKS / GVIEW", ui.compact() ? 15.0f : 22.0f, 11.0f);
    ui.label("main", "title", destination_name(model.destination), ui.compact() ? 38.0f : 64.0f,
             ui.compact() ? 28.0f : 42.0f);
    ui.container("main", "content", glayout::ContainerKind::Column,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Fill, 1.0f}, 10.0f);
    if (ui.phone()) ui.scrolling("content");

    if (model.provider_state != "Populated") {
        const std::string status =
            model.provider_state == "Loading"
                ? "LOADING PROVIDER\nResolving profiles, saves, packages, and "
                  "network state…"
            : model.provider_state == "Empty"
                ? "NO DATA YET\nThis provider has no entries. The normal shell "
                  "remains navigable."
                : "PROVIDER UNAVAILABLE\nThe game adapter returned an error. Retry "
                  "or continue offline.";
        ui.label("content", "provider-state", status, 150.0f, 20.0f, gview::TextAlign::Center);
        ui.button("content", "provider-retry",
                  model.provider_state == "Empty" ? "+ Create first entry" : "Retry",
                  "toast:Provider request queued", "provider", 48.0f);
        ui.focus_group("provider", "provider-retry", active_nav(model));
        ui.edge(active_nav(model), gview::NavAction::Right, "provider-retry");
    } else switch (model.destination) {
        case Destination::Play:
            build_play(ui, model, "content");
            break;
        case Destination::Players:
            build_players(ui, model, "content");
            break;
        case Destination::Settings:
            build_settings(ui, model, "content");
            break;
        case Destination::Controls:
            build_controls(ui, model, "content");
            break;
        case Destination::Progress:
            build_progress(ui, model, "content");
            break;
        case Destination::Mods:
            build_mods(ui, model, "content");
            break;
        }

    if (ui.compact()) {
        ui.container("shell-frame", "mobile-nav", glayout::ContainerKind::Row,
                     {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Pixels, 54.0f}, 2.0f,
                     {4.0f, 2.0f, 4.0f, 2.0f});
        for (const Nav& entry : entries) {
            const std::string id = std::string("nav-") + entry.name;
            ui.button("mobile-nav", id, entry.name, std::string("destination:") + entry.name,
                      "rail", 50.0f, gview::ActivationPolicy::OnFocus);
            ui.spec(id).text_style.size = 10.0f;
            ui.spec(id).text_style.horizontal = gview::TextAlign::Center;
            ui.spec(id).selected = id == active_nav(model);
        }
    } else {
        ui.container("shell-frame", "footer", glayout::ContainerKind::Row,
                     {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Pixels, 30.0f}, 16.0f,
                     {18.0f, 0.0f, 18.0f, 0.0f});
        ui.label("footer", "help",
                 "D-pad / left stick  Navigate    Enter / A  Select    Esc / B  Back", 30.0f,
                 11.0f);
        ui.label("footer", "focus-status", "●  Native focus graph active", 30.0f, 11.0f,
                 gview::TextAlign::End);
    }
    ui.focus_group("rail", active_nav(model));
    if (!model.modal.empty()) {
        ui.container("root", "modal-layer", glayout::ContainerKind::Stack,
                     {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Fill, 1.0f});
        ui.spec("modal-layer").style.normal.fill = {0, 0, 0, 190};
        ui.container("modal-layer", "modal-card", glayout::ContainerKind::Column,
                     {glayout::LengthKind::Pixels, 520.0f}, {glayout::LengthKind::Pixels, 274.0f},
                     10.0f, {22.0f, 18.0f, 22.0f, 18.0f});
        ui.layout("modal-card").align = glayout::Align::Center;
        ui.label("modal-card", "modal-kicker", "CONFIRM CHANGE", 24.0f, 11.0f);
        ui.label("modal-card", "modal-title", model.modal, 48.0f, 25.0f);
        ui.label("modal-card", "modal-copy",
                 "Gubsy computed the complete dependency and state impact. This "
                 "local trial does not mutate files.",
                 74.0f, 14.0f);
        ui.container("modal-card", "modal-actions", glayout::ContainerKind::Row,
                     {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Pixels, 48.0f}, 8.0f);
        ui.button("modal-actions", "modal-cancel", "Cancel", "modal:cancel", "modal", 48.0f);
        ui.button("modal-actions", "modal-confirm", "Confirm", "modal:confirm", "modal", 48.0f);
        ui.focus_group("modal", "modal-cancel");
    }
    if (!model.toast.empty()) {
        ui.container("root", "toast-layer", glayout::ContainerKind::Overlay,
                     {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Fill, 1.0f});
        ui.spec("toast-layer").style.normal.fill = {0, 0, 0, 0};
        ui.button("toast-layer", "toast-message", model.toast + "    ×", "toast:clear", "toast",
                  48.0f);
        ui.layout("toast-message").absolute_rect = {0.34f, 0.89f, 0.32f, 0.07f};
        ui.spec("toast-message").stratum = gview::PaintStratum::Prompt;
    }
    gview::View view = ui.finish();
    if (!model.modal.empty()) {
        for (gview::NodeSpec& spec : view.nodes) {
            const bool modal_node = spec.layout_id.rfind("modal-", 0) == 0;
            if (modal_node) spec.stratum = gview::PaintStratum::Modal;
            else spec.focusable = false;
        }
    }
    return view;
}
