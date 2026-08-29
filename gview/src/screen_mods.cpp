#include "view_builder.hpp"

#include <algorithm>
#include <array>

namespace {

struct Mod {
    const char* name;
    const char* detail;
    bool compatible;
};

constexpr std::array<Mod, 20> catalog{{
    {"Mycelium Below", "Worlds · Biomes · Co-op · 18.8k downloads · 96% positive", true},
    {"Brassline Grapple Kit", "Mechanics · Traversal · 31.2k · requires Cartographer's Desk", true},
    {"Skybreak Caverns", "Worlds · Sky · 12.4k · requires game 1.4", true},
    {"Abyssal Tide", "Worlds · Water · 17.2k · requires Underground Rivers", true},
    {"Mirror Depths", "Quest · Challenge · 19.8k · requires Mod API 0.2", false},
    {"Ember Orchard", "Biome · Items · 8.1k · requires Base Content", true},
    {"Clockwork Descent", "Quest · Machines · 27.7k · requires game 1.4", true},
    {"Fossil Gardens", "Biome · Creatures · 11.9k · requires Mycelium Below", true},
    {"Moonlit Markets", "Shops · Items · 42.3k · requires game 1.3", true},
    {"Coral Reliquary", "Quest · Water · 6.4k · requires API 0.3 preview", false},
    {"Iron Root Arsenal", "Items · Combat · 23.6k · requires Base Content", true},
    {"Echoing Markets", "Audio · Shops · 14.0k · requires Moonlit Markets", true},
    {"Silken Catacombs", "Worlds · Traps · 9.7k · requires game 1.4", true},
    {"Friendly Ghosts", "Creatures · Co-op · 38.5k · 99% positive", true},
    {"Temple Weather", "Effects · World · 21.1k · requires renderer 2.0", false},
    {"Campfire Stories", "Quests · Dialogue · 5.8k · requires Old Lanterns", true},
    {"The Violet Vault", "Quest · Endgame · 29.2k · requires Violet Reach", true},
    {"Tiny Expeditions", "Mode · Casual · 33.0k · requires game 1.4", true},
    {"Relay Races", "Mode · Co-op · 15.5k · requires networking 1.2", true},
    {"Legacy Room Pack", "Rooms · Archive · 4.2k · built for game 0.9 only", false},
}};

constexpr std::array<const char*, 7> installed_mods{
    "Base Content", "Cartographer's Desk", "Pocket Expedition", "Old Lanterns",
    "Underground Rivers", "Mycelium Below", "Temple Weather"};

// What is: Presentation data derived from the selected host-owned package.
std::string mod_asset(std::string_view name, bool installed) {
    if (installed) {
        const auto found = std::find(installed_mods.begin(), installed_mods.end(), name);
        const std::size_t index = found == installed_mods.end() ? 0 : found - installed_mods.begin();
        return "mod-" + std::to_string(index);
    }
    const auto found = std::find_if(catalog.begin(), catalog.end(),
                                    [&](const Mod& mod) { return mod.name == name; });
    const std::size_t index = found == catalog.end() ? 0 : found - catalog.begin();
    return "mod-" + std::to_string(index);
}

std::string mod_summary(std::string_view name, bool installed) {
    if (installed)
        return "Installed package " + std::string(name) +
               " is part of this device's versioned content graph. Dependencies and dependents "
               "are checked before changes are applied.";
    const auto found = std::find_if(catalog.begin(), catalog.end(),
                                    [&](const Mod& mod) { return mod.name == name; });
    return found == catalog.end() ? std::string{} : found->detail;
}

void tabs(ViewBuilder& ui, std::string_view content, const TrialModel& model) {
    ui.container(content, "mod-tabs", glayout::ContainerKind::Row,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Pixels, 44.0f}, 4.0f);
    for (const char* tab : {"Installed", "Browse catalog"})
        ui.button("mod-tabs", std::string("mod-tab-") + tab, tab, std::string("mods-tab:") + tab,
                  "mod-tabs", 44.0f, gview::ActivationPolicy::OnFocus);
    ui.spec(std::string("mod-tab-") + model.mods_tab).selected = true;
    ui.focus_group("mod-tabs", std::string("mod-tab-") + model.mods_tab, "nav-Mods");
}

void detail(ViewBuilder& ui, std::string_view parent, bool installed, const TrialModel& model) {
    ui.container(parent, "mod-detail", glayout::ContainerKind::Column,
                 {glayout::LengthKind::Pixels, 440.0f}, {glayout::LengthKind::Fill, 1.0f}, 7.0f,
                 {14.0f, 12.0f, 14.0f, 12.0f});
    ui.scrolling("mod-detail");
    ui.spec("mod-detail").style_class = "panel";
    ui.image("mod-detail", "mod-hero", mod_asset(model.selected_mod, installed), 132.0f);
    ui.label("mod-detail", "mod-kicker", installed ? "INSTALLED PACKAGE" : "CATALOG ENTRY", 22.0f,
             11.0f);
    ui.label("mod-detail", "mod-name", model.selected_mod, 42.0f, 25.0f);
    ui.label("mod-detail", "mod-copy", mod_summary(model.selected_mod, installed), 66.0f, 13.0f);
    if (installed)
        ui.select("mod-detail", "mod-version", "Selected version", "mod-version",
                  {"v1.2.0", "v1.3.1", "v1.4.0", "Latest compatible"}, "mods-content", 50.0f);
    const std::string graph =
        installed ? "PACKAGE GRAPH\nSelected       " + model.selected_mod +
                        "\nStatus         INSTALLED\nDependencies   SATISFIED\nDependents     "
                        "CHECKED BEFORE CHANGE"
                  : "INSTALL PLAN\nSelected       " + model.selected_mod +
                        "\nVersion        v1.1.0\nDownload       84.2 MB\nDependencies   2 "
                        "packages\nAUTOMATIC DEPENDENCIES\n↳ Base Content       INSTALLED\n↳ "
                        "Underground Rivers  WILL INSTALL";
    ui.label("mod-detail", "dependency-graph", graph,
             installed ? 144.0f : 156.0f, 12.0f);
    if (installed) {
        ui.label("mod-detail", "uninstall-warning",
                 "Cascading uninstall is guarded. A dependency plan is shown before removal.",
                 48.0f, 12.0f);
        ui.container("mod-detail", "installed-actions", glayout::ContainerKind::Row,
                     {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Pixels, 46.0f}, 6.0f);
        ui.button("installed-actions", "update-mod", "Update", "toast:Update planned",
                  "mods-content", 46.0f);
        ui.button("installed-actions", "open-files", "Open files", "toast:Folder opened",
                  "mods-content", 46.0f);
        ui.button("installed-actions", "uninstall-mod", "Uninstall…", "modal:uninstall",
                  "mods-content", 46.0f);
    } else {
        ui.button("mod-detail", "install-session", "Install & add to current session",
                  "toast:Installed and added", "mods-content", 48.0f);
        ui.button("mod-detail", "install-only", "Install only", "toast:Installed", "mods-content",
                  48.0f);
    }
}

void installed(ViewBuilder& ui, std::string_view content, const TrialModel& model) {
    ui.container(content, "installed-toolbar", glayout::ContainerKind::Row,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Pixels, 46.0f}, 8.0f);
    ui.label("installed-toolbar", "installed-summary",
             "ON THIS DEVICE · 7 installed · game build 1.4", 46.0f, 13.0f);
    ui.button("installed-toolbar", "refresh-installed", "↻ Refresh", "toast:Catalog refreshed",
              "mods-content", 46.0f);
    ui.container(content, "installed-workspace", ui.split(), {glayout::LengthKind::Fill, 1.0f},
                 {glayout::LengthKind::Fill, 1.0f}, 12.0f);
    ui.container("installed-workspace", "installed-list", glayout::ContainerKind::Column,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Fill, 1.0f}, 5.0f,
                 {12.0f, 10.0f, 12.0f, 10.0f});
    ui.scrolling("installed-list");
    ui.spec("installed-list").style_class = "panel";
    for (std::size_t index = 0; index < installed_mods.size(); ++index) {
        const std::string id = "installed-" + std::to_string(index);
        ui.button("installed-list", id,
                  installed_mods[index] + std::string("\nInstalled · dependencies satisfied"),
                  std::string("mod-select:") + installed_mods[index], "mod-list", 58.0f,
                  gview::ActivationPolicy::OnFocus);
        ui.spec(id).selected = model.selected_mod == installed_mods[index];
    }
    detail(ui, "installed-workspace", true, model);
}

void browse(ViewBuilder& ui, std::string_view content, const TrialModel& model) {
    ui.container(content, "catalog-tools", glayout::ContainerKind::Row,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Pixels, 48.0f}, 8.0f);
    ui.text_input("catalog-tools", "catalog-search", "Search the Gubsy mod catalog…",
                  "catalog-search", "mods-content", 48.0f);
    ui.toggle("catalog-tools", "compatible", "Compatible only", "compatible-only", "mods-content",
              48.0f);
    ui.button("catalog-tools", "catalog-refresh", "↻ Refresh", "toast:Catalog refreshed",
              "mods-content", 48.0f);
    ui.container(content, "catalog-workspace", ui.split(), {glayout::LengthKind::Fill, 1.0f},
                 {glayout::LengthKind::Fill, 1.0f}, 12.0f);
    ui.container("catalog-workspace", "catalog-list", glayout::ContainerKind::Column,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Fill, 1.0f}, 4.0f,
                 {12.0f, 10.0f, 12.0f, 10.0f});
    ui.layout("catalog-list").clip = true;
    ui.spec("catalog-list").control = gview::ControlKind::ScrollArea;
    ui.spec("catalog-list").style_class = "panel";
    ui.label("catalog-list", "catalog-count", "20 CATALOG MODS", 24.0f, 11.0f);
    for (std::size_t index = 0; index < catalog.size(); ++index) {
        ui.container("catalog-list", "catalog-row-" + std::to_string(index),
                     glayout::ContainerKind::Row, {glayout::LengthKind::Fill, 1.0f},
                     {glayout::LengthKind::Pixels, 66.0f}, 8.0f);
        ui.image("catalog-row-" + std::to_string(index), "catalog-image-" + std::to_string(index),
                 "mod-" + std::to_string(index), 66.0f);
        ui.layout("catalog-image-" + std::to_string(index)).size.width = {
            glayout::LengthKind::Pixels, 78.0f * ui.scale()};
        const std::string id = "catalog-item-" + std::to_string(index);
        ui.button("catalog-row-" + std::to_string(index), id,
                  std::string(catalog[index].name) + "\n" + catalog[index].detail +
                      (catalog[index].compatible ? "" : " · INCOMPATIBLE"),
                  std::string("mod-select:") + catalog[index].name, "mod-list", 66.0f,
                  gview::ActivationPolicy::OnFocus);
        ui.spec(id).selected = model.selected_mod == catalog[index].name;
    }
    detail(ui, "catalog-workspace", false, model);
}

} // namespace

void build_mods(ViewBuilder& ui, const TrialModel& model, std::string_view content) {
    tabs(ui, content, model);
    const std::string owner = std::string("mod-tab-") + model.mods_tab;
    if (model.mods_tab == "Browse catalog") {
        browse(ui, content, model);
        ui.focus_scope("catalog-tools", "mod-tools");
        ui.focus_scope("catalog-list", "mod-list");
        ui.focus_scope("mod-detail", "mod-detail");
        ui.focus_group("mod-tools", "catalog-search", owner);
        ui.focus_group("mod-list", "catalog-item-0", owner);
        ui.focus_group("mod-detail", "install-session", owner);
    } else {
        installed(ui, content, model);
        ui.focus_scope("installed-toolbar", "mod-tools");
        ui.focus_scope("installed-list", "mod-list");
        ui.focus_scope("mod-detail", "mod-detail");
        ui.focus_group("mod-tools", "refresh-installed", owner);
        ui.focus_group("mod-list", "installed-0", owner);
        ui.focus_group("mod-detail", "mod-version", owner);
    }
    ui.edge("nav-Mods", gview::NavAction::Right, std::string("mod-tab-") + model.mods_tab);
    ui.group_edge("mod-tabs", gview::NavAction::Left, "rail");
    ui.group_edge("mod-tabs", gview::NavAction::Down, "mod-tools");
    ui.group_edge("mod-tools", gview::NavAction::Up, "mod-tabs");
    ui.group_edge("mod-tools", gview::NavAction::Down, "mod-list");
    ui.group_edge("mod-list", gview::NavAction::Up, "mod-tools");
    ui.group_edge("mod-list", gview::NavAction::Right, "mod-detail");
    ui.group_edge("mod-list", gview::NavAction::Confirm, "mod-detail");
    ui.group_edge("mod-detail", gview::NavAction::Left, "mod-list");
    ui.group_edge("mod-detail", gview::NavAction::Back, "mod-list");
    ui.group_edge("mod-detail", gview::NavAction::Up, "mod-tabs");
}
