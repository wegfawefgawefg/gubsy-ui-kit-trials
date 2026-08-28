#include "view_builder.hpp"

void build_progress(ViewBuilder& ui, const TrialModel&, std::string_view content) {
    ui.container(content, "progress-toolbar", glayout::ContainerKind::Row,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Pixels, 48.0f}, 8.0f);
    ui.label("progress-toolbar", "progress-summary", "GAME-PROVIDED PROGRESSION · 3 campaigns",
             48.0f, 13.0f);
    ui.button("progress-toolbar", "new-quest", "+ Choose a new quest", "play:quest",
              "progress", 48.0f);
    ui.container(content, "progress-workspace", ui.split(),
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Fill, 1.0f}, 12.0f);
    ui.container("progress-workspace", "campaign-list", glayout::ContainerKind::Column,
                 {glayout::LengthKind::Pixels, 440.0f}, {glayout::LengthKind::Fill, 1.0f}, 7.0f,
                 {12.0f, 10.0f, 12.0f, 10.0f});
    ui.label("campaign-list", "campaign-title", "CAMPAIGNS", 28.0f, 11.0f);
    ui.button("campaign-list", "campaign-glass", "The Glass Caverns\nMoss · Temple Depths · 42%       READY",
              "select:The Glass Caverns", "progress", 72.0f);
    ui.button("campaign-list", "campaign-quiet", "A Quiet Beginning\nVega · Green Valley · 11%       READY",
              "select:A Quiet Beginning", "progress", 72.0f);
    ui.button("campaign-list", "campaign-old", "Old Expedition\nMoss · Version 0.7 data       INCOMPATIBLE",
              "select:Old Expedition", "progress", 72.0f);
    ui.container("progress-workspace", "campaign-detail", glayout::ContainerKind::Column,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Fill, 1.0f}, 7.0f,
                 {14.0f, 12.0f, 14.0f, 12.0f});
    ui.label("campaign-detail", "campaign-name", "CAMPAIGN · PLAY\nThe Glass Caverns", 58.0f, 24.0f);
    ui.label("campaign-detail", "campaign-owner", "MO  ASSOCIATED PROFILE\n     Moss · ownership recorded by provider",
             58.0f, 13.0f);
    ui.label("campaign-detail", "campaign-mods",
             "RECORDED MOD SET · 7 PACKAGES\nBase Content                  v1.4.0\nCartographer's Desk           v0.8.2\nOld Lanterns                  Update available\nManifest hash                 8cc91a",
             132.0f, 13.0f);
    ui.button("campaign-detail", "inspect-packages", "Inspect exact package set",
              "play:mods", "progress", 46.0f);
    ui.label("campaign-detail", "checkpoint-title", "CHECKPOINT HISTORY", 25.0f, 11.0f);
    ui.button("campaign-detail", "checkpoint-temple", "Temple safe room · Today 06:52 · RESUME POINT",
              "toast:Checkpoint selected", "progress", 46.0f);
    ui.button("campaign-detail", "checkpoint-flooded", "Flooded archive · Yesterday · BACKUP",
              "toast:Checkpoint selected", "progress", 46.0f);
    ui.button("campaign-detail", "checkpoint-mushroom", "Mushroom crossing · Aug 24 · BACKUP",
              "toast:Checkpoint selected", "progress", 46.0f);
    ui.container("campaign-detail", "campaign-actions", glayout::ContainerKind::Row,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Pixels, 48.0f}, 7.0f);
    ui.button("campaign-actions", "resume-campaign", "▶ Resume campaign", "start-session",
              "progress", 48.0f);
    ui.button("campaign-actions", "export-campaign", "Export", "toast:Campaign exported",
              "progress", 48.0f);
    ui.button("campaign-actions", "delete-campaign", "Delete", "modal:delete",
              "progress", 48.0f);
    ui.focus_group("progress", "new-quest", "nav-Progress");
    ui.edge("nav-Progress", gview::NavAction::Right, "new-quest");
}
