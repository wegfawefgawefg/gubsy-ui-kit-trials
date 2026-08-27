#include "app.h"
#include "ui_markup.h"

#include <sstream>

// Installed packages, catalog browsing, and dependency details.

std::string GubsyApp::BuildMods() const {
  std::ostringstream out;
  out << R"(<div class="local-tabs"><button data-focus data-action="mods-tab-Installed" class=")"
      << markup::selected_class(state_.mods_tab == "Installed")
      << R"(">Installed</button><button data-focus data-action="mods-tab-Browse catalog" class=")"
      << markup::selected_class(state_.mods_tab == "Browse catalog")
      << R"(">Browse catalog</button><span>LB / RB change section</span></div>)";
  if (state_.mods_tab == "Installed") {
    struct Mod {
      const char *action;
      const char *name;
      const char *author;
      const char *version;
      const char *state;
    };
    constexpr Mod mods[]{
        {"mod-base", "Base Content", "Gubsy", "v1.4.0", "COMPATIBLE"},
        {"mod-cartographer", "Cartographer's Desk", "Yarrow", "v0.8.2",
         "COMPATIBLE"},
        {"mod-pocket", "Pocket Expedition", "Mossbyte", "v0.9.1", "COMPATIBLE"},
        {"mod-lanterns", "Old Lanterns", "Palmeret", "v1.3.1", "UPDATE"},
        {"mod-rivers", "Underground Rivers", "Bathysphere", "v2.2.0",
         "COMPATIBLE"},
        {"mod-mycelium", "Mycelium Below", "Root & Branch", "v1.1.0",
         "COMPATIBLE"},
        {"mod-weather", "Temple Weather", "Northglass", "v3.0.0",
         "COMPATIBLE"}};
    out << R"(<div class="toolbar"><span class="toolbar-title"><small>ON THIS DEVICE</small>7 installed · game build 1.4</span><button data-focus data-action="refresh-mods" class="button">↻ Refresh</button></div><div class="master-detail"><section class="panel master-list scroll-list"><div class="scroll-body">)";
    for (const Mod &mod : mods) {
      const size_t image_index = static_cast<size_t>(&mod - mods);
      out << R"(<button data-focus data-action="installed-mod-)" << mod.name
          << R"(" class=")"
          << markup::selected_class(state_.selected_mod == mod.name, "mod-row")
          << R"("><img class="mod-thumb" src="../mods/)"
          << markup::mod_image(image_index) << R"("/><span><strong>)" << mod.name
          << R"(</strong><small>by )" << mod.author << R"( · )" << mod.version
          << R"(</small></span><b class=")"
          << (std::string(mod.state) == "UPDATE" ? "warning" : "active")
          << R"(">)" << mod.state << R"(</b></button>)";
    }
    out << R"(</div>)";
    out << R"(</section><aside class="panel detail mod-detail"><img class="hero-art" src="../mods/sheet-1-4.png"/><small>INSTALLED PACKAGE</small><h2>)"
        << state_.selected_mod
        << R"(</h2><p>Game-ready content with authored rooms, mechanics, and data supplied through the Gubsy package graph.</p><div class="form-row"><span>Selected version<small>Changing versions re-solves dependencies</small></span>)";
    markup::append_select(out, "choice-value-mod-version",
                  markup::map_value(state_.choice_values, "mod-version"),
                  {"v1.2.0", "v1.3.1", "v1.4.0", "Latest compatible"});
    out << R"(</div><div class="dependency"><h3>REQUIRES</h3><div><span>↳ Base Content ≥ 1.4</span><b>INSTALLED</b></div><div><span>↳ Underground Rivers ≥ 2.0</span><b>INSTALLED</b></div><h3>REQUIRED BY</h3><div><span>↑ Temple Weather</span><b>ACTIVE</b></div><div><span>↑ Pocket Expedition</span><b>ACTIVE</b></div></div><div class="warning-box"><strong>Cascading uninstall is guarded</strong><small>Removing this package also affects two installed dependents. Gubsy computes and presents the full change plan before mutation.</small></div><div class="actions mod-actions"><button data-focus data-action="update-mod" class="button primary">Update</button><button data-focus data-action="open-mod-folder" class="button">Open files</button><button data-focus data-action="uninstall-plan" class="button danger">Uninstall…</button></div></aside></div>)";
  } else {
    struct Catalog {
      const char *action;
      const char *name;
      const char *author;
      const char *category;
      const char *downloads;
      const char *rating;
      const char *requirement;
      bool compatible;
    };
    constexpr Catalog mods[]{
        {"mod-mycelium", "Mycelium Below", "Root & Branch",
         "Worlds · Biomes · Co-op", "18.8k", "96%", "Requires Base Content",
         true},
        {"mod-grapple", "Brassline Grapple Kit", "Clockwork Moss",
         "Mechanics · Traversal", "31.2k", "97%",
         "Requires Cartographer's Desk", true},
        {"mod-skybreak", "Skybreak Caverns", "Gale Assembly", "Worlds · Sky",
         "12.4k", "91%", "Requires game 1.4", true},
        {"mod-tide", "Abyssal Tide", "Bathysphere", "Worlds · Water", "17.2k",
         "93%", "Requires Underground Rivers", true},
        {"mod-depths", "Mirror Depths", "Parallax Union", "Quest · Challenge",
         "19.8k", "88%", "Requires Gubsy Mod API 0.2", false},
        {"mod-ember", "Ember Orchard", "Kindling Club", "Biome · Items", "8.1k",
         "94%", "Requires Base Content", true},
        {"mod-clock", "Clockwork Descent", "Ratchet House", "Quest · Machines",
         "27.7k", "98%", "Requires game 1.4", true},
        {"mod-fossil", "Fossil Gardens", "Marl & Fern", "Biome · Creatures",
         "11.9k", "90%", "Requires Mycelium Below", true},
        {"mod-moon", "Moonlit Markets", "Silver Cart", "Shops · Items", "42.3k",
         "95%", "Requires game 1.3", true},
        {"mod-coral", "Coral Reliquary", "Tideglass", "Quest · Water", "6.4k",
         "89%", "Requires API 0.3 preview", false},
        {"mod-iron", "Iron Root Arsenal", "Forge Choir", "Items · Combat",
         "23.6k", "92%", "Requires Base Content", true},
        {"mod-echo", "Echoing Markets", "Hollow Bell", "Audio · Shops", "14.0k",
         "96%", "Requires Moonlit Markets", true},
        {"mod-silk", "Silken Catacombs", "Threadbare", "Worlds · Traps", "9.7k",
         "87%", "Requires game 1.4", true},
        {"mod-ghost", "Friendly Ghosts", "Pale Picnic", "Creatures · Co-op",
         "38.5k", "99%", "Requires Base Content", true},
        {"mod-storm", "Temple Weather", "Northglass", "Effects · World",
         "21.1k", "94%", "Requires renderer 2.0", false},
        {"mod-camp", "Campfire Stories", "Wayfarer", "Quests · Dialogue",
         "5.8k", "92%", "Requires Old Lanterns", true},
        {"mod-vault", "The Violet Vault", "Archive Team", "Quest · Endgame",
         "29.2k", "97%", "Requires Violet Reach", true},
        {"mod-tiny", "Tiny Expeditions", "Pocket Moss", "Mode · Casual",
         "33.0k", "95%", "Requires game 1.4", true},
        {"mod-race", "Relay Races", "Split Second", "Mode · Co-op", "15.5k",
         "90%", "Requires networking 1.2", true},
        {"mod-legacy", "Legacy Room Pack", "Old Stone", "Rooms · Archive",
         "4.2k", "73%", "Built for game 0.9 only", false}};
    out << R"(<div class="catalog-tools"><input data-focus data-action="search-catalog" class="search-field" type="text" value=")"
        << markup::escape_attribute(state_.mod_filter)
        << R"(" placeholder="Search the Gubsy mod catalog…"/><label class="check-control"><input data-focus data-action="compatible-value" class="native-toggle" type="checkbox" value="true")";
    if (state_.compatible_only)
      out << R"( checked="checked")";
    out << R"(/><span>Compatible only<small>Hide mods unusable on this build</small></span></label><button data-focus data-action="refresh-catalog" class="button">↻ Refresh</button></div><div class="master-detail"><section class="panel master-list scroll-list catalog-list"><div class="scroll-body"><small>)"
        << (state_.compatible_only ? "COMPATIBLE MODS" : "20 CATALOG MODS")
        << R"(</small>)";
    for (const Catalog &mod : mods) {
      if (state_.compatible_only && !mod.compatible)
        continue;
      if (!markup::contains_ci(mod.name, state_.mod_filter) &&
          !markup::contains_ci(mod.author, state_.mod_filter) &&
          !markup::contains_ci(mod.category, state_.mod_filter) &&
          !markup::contains_ci(mod.requirement, state_.mod_filter))
        continue;
      const size_t image_index = static_cast<size_t>(&mod - mods);
      out << R"(<button data-focus data-action="catalog-mod-)" << mod.name
          << R"(" class=")"
          << markup::selected_class(state_.selected_mod == mod.name,
                            std::string("catalog-row ") +
                                (mod.compatible ? "" : "incompatible"))
          << R"("><img class="mod-thumb" src="../mods/)"
          << markup::mod_image(image_index) << R"("/><span><strong>)" << mod.name
          << R"(</strong><small>)" << mod.category << R"( · by )" << mod.author
          << R"( · )" << mod.downloads << R"( downloads</small><em>)"
          << mod.requirement << R"(</em></span><b>★ )" << mod.rating
          << R"( positive</b><i></i></button>)";
    }
    out << R"(</div>)";
    out << R"(</section><aside class="panel detail mod-detail"><img class="hero-art" src="../mods/sheet-1-0.png"/><div class="tag-row"><span>WORLDS</span><span>BIOMES</span><span>CO-OP</span></div><small>CATALOG ENTRY</small><h2>)"
        << state_.selected_mod
        << R"(</h2><p>A hand-built content package with carefully integrated rooms, mechanics, artwork, and native co-op synchronization.</p><div class="impact"><h3>INSTALL PLAN</h3><div class="kv"><span>Selected</span><b>v1.1.0</b></div><div class="kv"><span>Download</span><b>84.2 MB</b></div><div class="kv"><span>Dependencies</span><b>2 packages</b></div><div class="kv"><span>Compatibility</span><b>Game 1.4 ready</b></div></div><div class="dependency"><h3>AUTOMATIC DEPENDENCIES</h3><div><span>↳ Base Content v1.4.0</span><b>INSTALLED</b></div><div><span>↳ Underground Rivers v2.2.0</span><b>WILL INSTALL</b></div></div><button data-focus data-action="install-add-session" class="button primary">Install &amp; add to current session</button><button data-focus data-action="install-only" class="button">Install only</button></aside></div>)";
  }
  return out.str();
}
