#include "view_builder.hpp"

#include <array>

// Proves that GLayout/GView can compose an in-game inventory over native game rendering.
gview::View build_game_ui_view(const TrialModel& model, int width, int height) {
    ViewBuilder ui("inventory-overlay", width, height);
    const float s = ui.scale();
    ui.surface("root", "world-preview", "world-preview");
    ui.container("root", "inventory-safe", glayout::ContainerKind::Column,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Fill, 1.0f}, 10.0f,
                 {30.0f, 20.0f, 30.0f, 18.0f});
    ui.spec("inventory-safe").style.normal.fill = {2, 12, 17, 96};
    ui.container("inventory-safe", "inventory-header", glayout::ContainerKind::Row,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Pixels, 74.0f}, 12.0f);
    ui.label("inventory-header", "hud-health", ui.phone() ? "♥ ♥ ♥" : "♥ ♥ ♥   THE VIOLET REACH",
             74.0f, ui.phone() ? 12.0f : 16.0f);
    ui.label("inventory-header", "inventory-title", "INVENTORY", 74.0f, ui.phone() ? 18.0f : 28.0f,
             gview::TextAlign::Center);
    ui.label("inventory-header", "hud-currency", "◇  1,464", 74.0f, ui.phone() ? 12.0f : 18.0f,
             gview::TextAlign::End);

    ui.container("inventory-safe", "inventory-tabs", glayout::ContainerKind::Row,
                 {glayout::LengthKind::Percent, 0.58f}, {glayout::LengthKind::Pixels, 52.0f}, 5.0f);
    constexpr std::array tabs{"TOOLS", "RELICS", "SUPPLIES", "KEYS", "FAVORITES"};
    for (std::size_t index = 0; index < tabs.size(); ++index) {
        const std::string id = "inventory-tab-" + std::to_string(index);
        ui.button("inventory-tabs", id, tabs[index], "toast:Inventory category", "inventory-tabs",
                  52.0f, gview::ActivationPolicy::OnFocus);
        ui.spec(id).selected = index == 2;
        ui.spec(id).text_style.horizontal = gview::TextAlign::Center;
        ui.spec(id).text_style.size = 12.0f * s;
    }

    ui.container("inventory-safe", "inventory-body", ui.split(), {glayout::LengthKind::Fill, 1.0f},
                 {glayout::LengthKind::Fill, 1.0f}, 18.0f);
    ui.container("inventory-body", "inventory-grid", glayout::ContainerKind::Grid,
                 ui.phone() ? glayout::Length{glayout::LengthKind::Fill, 1.0f}
                            : glayout::Length{glayout::LengthKind::Percent, 0.58f},
                 {glayout::LengthKind::Fill, 1.0f}, 7.0f, {8.0f, 8.0f, 8.0f, 8.0f});
    ui.layout("inventory-grid").columns = width < 900 ? 4 : 6;
    ui.layout("inventory-grid").clip = true;
    ui.scrolling("inventory-grid");
    ui.spec("inventory-grid").style.normal.fill = {3, 18, 22, 218};
    for (int index = 0; index < 24; ++index) {
        const std::string id = "inventory-item-" + std::to_string(index);
        ui.button("inventory-grid", id, "×" + std::to_string(index % 9 + 1),
                  "select:inventory-" + std::to_string(index), "inventory", 92.0f);
        ui.spec(id).asset = "mod-" + std::to_string(index % 20);
        ui.spec(id).content = gview::ContentKind::Image;
        ui.spec(id).selected = index == 8;
        ui.spec(id).style.selected.border_width = 3.0f;
        ui.spec(id).style.selected_focused.border_width = 3.0f;
    }
    ui.container("inventory-body", "inventory-detail", glayout::ContainerKind::Column,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Fill, 1.0f}, 8.0f,
                 {16.0f, 14.0f, 16.0f, 14.0f});
    ui.spec("inventory-detail").style.normal.fill = {3, 18, 22, 218};
    if (!ui.compact()) ui.image("inventory-detail", "item-preview", "mod-8", 150.0f);
    ui.label("inventory-detail", "item-name", "COURSIER BEE HONEY", ui.compact() ? 32.0f : 42.0f,
             ui.compact() ? 19.0f : 26.0f);
    ui.label("inventory-detail", "item-stats", "♥ ♥    RESTORES 4 HEALTH", 26.0f, 13.0f);
    ui.label("inventory-detail", "item-copy",
             "Honey gathered below the Glass Caverns. Eat it now, hold it for crafting, or move it "
             "into the shared party pack.",
             ui.compact() ? 46.0f : 70.0f, ui.compact() ? 12.0f : 15.0f);
    ui.container("inventory-detail", "item-actions",
                 ui.compact() ? glayout::ContainerKind::Row : glayout::ContainerKind::Column,
                 {glayout::LengthKind::Fill, 1.0f},
                 {glayout::LengthKind::Pixels, ui.compact() ? 34.0f : 112.0f}, 6.0f);
    ui.button("item-actions", "item-use", "Use", "toast:Item used", "inventory-actions", 32.0f);
    ui.button("item-actions", "item-hold", "Hold", "toast:Item held", "inventory-actions", 32.0f);
    ui.button("item-actions", "item-cancel", "Cancel", "back", "inventory-actions", 32.0f);
    ui.label("inventory-safe", "inventory-help",
             "LB / RB  Category     D-pad  Select     A  Actions     B  Close     Y  Sort", 30.0f,
             12.0f, gview::TextAlign::Center);
    ui.focus_group("inventory-tabs", "inventory-tab-2");
    ui.focus_group("inventory", "inventory-item-8", "inventory-tab-2");
    ui.focus_group("inventory-actions", "item-use", "inventory-item-8");
    ui.edge("inventory-tab-2", gview::NavAction::Down, "inventory-item-8");
    ui.edge("inventory-item-8", gview::NavAction::Right, "item-use");
    if (!model.toast.empty()) {
        ui.container("root", "toast-layer", glayout::ContainerKind::Overlay,
                     {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Fill, 1.0f});
        ui.spec("toast-layer").style.normal.fill = {0, 0, 0, 0};
        ui.button("toast-layer", "toast-message", model.toast + "    ×", "toast:clear", "toast",
                  44.0f);
        ui.layout("toast-message").absolute_rect = {0.36f, 0.86f, 0.28f, 0.07f};
        ui.spec("toast-message").stratum = gview::PaintStratum::Prompt;
    }
    return ui.finish();
}
