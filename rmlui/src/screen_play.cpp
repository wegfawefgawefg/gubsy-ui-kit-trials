#include "app.h"
#include "ui_markup.h"

#include <sstream>

// Lobby, quest, rule, and session-package surfaces.

std::string GubsyApp::BuildPlayLobby() const {
  std::ostringstream out;
  const bool continuing = state_.activity == "Continue expedition";
  const bool arena = state_.activity == "Arena run";
  const char *activity_label =
      continuing ? "CONTINUE QUEST"
                 : (arena ? "ARENA ACTIVITY" : "NEW EXPEDITION");
  const char *action_label =
      continuing ? "▶ Resume latest checkpoint"
                 : (arena ? "▶ Enter arena lobby" : "▶ Begin expedition");
  out << R"(<div class="mobile-local-tabs"><button data-focus data-action="show-setup" class=")"
      << (state_.party_pane ? "" : "selected")
      << R"(">Setup</button><button data-focus data-action="show-party" class=")"
      << (state_.party_pane ? "selected" : "")
      << R"(">Party · 1/4</button></div><div class="workspace lobby-workspace">)";
  out << R"(<section class="panel setup-pane )"
      << (state_.party_pane ? "phone-hidden" : "")
      << R"("><div class="quest-row"><img class="quest-art" src="../splonks-title.png"/><div><small>)"
      << activity_label << R"(</small><h2>)"
      << (arena ? "The Relay Pit" : state_.selected_quest) << R"(</h2><p>)"
      << (continuing ? "The Glass Caverns · "
                     : (arena ? "Round set · " : "Quest route · "))
      << (continuing ? state_.selected_checkpoint
                     : (arena ? "Competitive rules" : "Stage one"))
      << R"( · Vega</p></div><button data-focus data-action="play-quest" class="button compact">)"
      << (continuing ? "Checkpoint" : (arena ? "Arena" : "Quest"))
      << R"(</button></div>)";
  out << R"(<div class="option-row"><span><strong>Activity</strong><small>Splonks supplies a different session model for each activity</small></span>)";
  markup::append_select(out, "play-value-activity", state_.activity,
                {"Continue expedition", "New expedition", "Arena run"});
  out << R"(</div>)";
  out << R"(<button data-focus data-action="play-quest" class="option-row command"><span><strong>)"
      << (continuing ? "Resume point"
                     : (arena ? "Arena rotation" : "Starting quest"))
      << R"(</strong><small>)"
      << (continuing
              ? state_.selected_checkpoint
              : (arena ? "Relay Pit · best of five" : state_.selected_quest))
      << R"(</small></span><b>)"
      << (continuing ? "CHOOSE CHECKPOINT ›" : "CHANGE ›")
      << R"(</b></button>)";
  out << R"(<div class="option-row"><span><strong>Play with</strong><small>Who may occupy the remaining slots</small></span>)";
  markup::append_select(out, "play-value-access", state_.access,
                {"Solo", "Friends can join", "Invite only", "Public"});
  out << R"(</div><div class="option-row"><span><strong>Host using</strong><small>Automatic chooses the best available route</small></span>)";
  markup::append_select(out, "play-value-host", state_.host,
                {"Automatic", "Host locally", "Dedicated relay"});
  out << R"(</div>)";
  out << R"(<button data-focus data-action="play-rules" class="option-row command"><span><strong>Expedition rules</strong><small>Standard · )"
      << markup::map_value(state_.rule_values, "shared-lives") << " lives · ghost at "
      << markup::map_value(state_.rule_values, "ghost-arrival")
      << R"(s</small></span><b>EDIT ALL ›</b></button>)";
  out << R"(<button data-focus data-action="play-mods" class="option-row command"><span><strong>Session mods</strong><small>7 active · differs from checkpoint</small></span><b>MANAGE ›</b></button>)";
  out << R"(<div class="actions"><button data-focus data-action="pause-preview" class="button">Pause preview</button><button data-focus data-action="start-session" class="button primary">)"
      << action_label << R"(</button></div></section>)";
  out << R"(<aside class="panel party-pane )"
      << (!state_.party_pane ? "phone-hidden" : "")
      << R"("><div class="panel-heading"><div><small>PLAYERS</small><h2>Your party</h2></div><b>OFFLINE</b></div><div class="player"><em>P1</em><span><strong>Moss</strong><small>Xbox Wireless Controller</small></span><b>READY</b></div>)";
  for (int i = 0; i < 3; ++i)
    out << R"(<button data-focus data-action="add-player" class="open-slot">+<span>Open slot<small>Invite a friend or add locally</small></span></button>)";
  out << R"(<div class="pair"><button data-focus data-action="copy-link" class="button">Invite / copy link</button><button data-focus data-action="find-games" class="button">Friends &amp; public games</button></div><dl class="summary"><dt>CONTENT</dt><dd>7 mods</dd><dt>RULESET</dt><dd>Standard</dd><dt>NETWORK</dt><dd>Automatic</dd></dl></aside></div>)";
  return out.str();
}

std::string GubsyApp::BuildQuestPicker() const {
  const bool continuing = state_.activity == "Continue expedition";
  const bool arena = state_.activity == "Arena run";
  struct Quest {
    const char *action;
    const char *name;
    const char *region;
    const char *stages;
  };
  constexpr Quest quests[]{
      {"quest-violet", "The Violet Reach", "Temple frontier · Moderate", "6"},
      {"quest-sunken", "The Sunken Road", "River caverns · Hard", "5"},
      {"quest-green", "A Green Beginning", "Valley underworks · Welcoming",
       "4"},
      {"quest-glass", "The Glass Pilgrim", "Crystal descent · Severe", "7"}};
  std::ostringstream out;
  out << R"(<div class="subview-header panel"><button data-focus data-action="play-lobby" class="button">‹ Back to lobby</button><div><small>SPLONKS QUEST PROVIDER</small><h2>)"
      << (continuing ? "Choose a resume point"
                     : (arena ? "Choose an arena rotation" : "Choose a quest"))
      << R"(</h2><p>)"
      << (continuing ? "Continue data selects a quest, its owning profile, and "
                       "a checkpoint inside that quest."
                     : (arena ? "Arena activities choose competitive maps and "
                                "round structure without campaign checkpoints."
                              : "A new expedition begins at stage one of the "
                                "selected quest with a fresh run manifest."))
      << R"(</p></div></div><div class="master-detail"><section class="panel master-list"><small>)"
      << (arena ? "ARENA ROTATIONS" : "EXPEDITIONS AND QUESTS")
      << R"(</small>)";
  for (const Quest &quest : quests) {
    out << R"(<button data-focus data-action=")" << quest.action
        << R"(" class=")"
        << markup::selected_class(state_.selected_quest == quest.name, "list-row")
        << R"("><em>)" << quest.stages << R"(</em><span><strong>)" << quest.name
        << R"(</strong><small>)" << quest.region
        << R"(</small></span><b>QUEST</b></button>)";
  }
  out << R"(</section><article class="panel detail"><button data-focus data-action="play-lobby" class="mobile-back">‹ Quest choices</button><small>TEMPLE FRONTIER</small><h2>)"
      << state_.selected_quest
      << R"(</h2><p>Follow a fractured relay signal through flooded archives, fungal crossings, and a temple complex waking beneath the mountain.</p><div class="tag-row"><span>6 stages</span><span>Moderate</span><span>Latest checkpoint</span></div><h3>QUEST ROUTE</h3><div class="route">)";
  constexpr const char *stages[]{"North Pass",      "Mushroom Crossing",
                                 "Flooded Archive", "Temple Gate",
                                 "The Relay",       "Violet Core"};
  for (int i = 0; i < 6; ++i)
    out << R"(<div class=")" << (i < 4 ? "reached" : "") << R"("><b>)" << i + 1
        << R"(</b><span>)" << stages[i] << R"(</span></div>)";
  out << R"(</div><div class="context-box"><strong>)"
      << (continuing ? "Checkpoint payload"
                     : (arena ? "Arena manifest" : "New expedition manifest"))
      << R"(</strong><p>)"
      << (continuing
              ? "Inventory, quest flags, world seed, owning profile, and exact "
                "mod versions come from this checkpoint."
              : (arena ? "Map rotation, round rules, players, and session mods "
                         "are fixed when the arena lobby launches."
                       : "Quest, seed policy, expedition rules, players, and "
                         "session mods are recorded when the new run begins."))
      << R"(</p></div><button data-focus data-action="play-lobby" class="button primary">)"
      << (continuing ? "Use this checkpoint"
                     : (arena ? "Use this rotation" : "Start with this quest"))
      << R"(</button></article></div>)";
  return out.str();
}

std::string GubsyApp::BuildRules() const {
  struct Rule {
    const char *id;
    const char *name;
    const char *note;
    const char *kind;
    const char *source;
  };
  constexpr Rule rules[]{
      {"difficulty", "Expedition difficulty",
       "Overall danger and resource pressure.", "select", ""},
      {"variation", "Stage variation", "How authored rooms are remixed.",
       "select", ""},
      {"shared-lives", "Shared lives", "Continues available to the party.",
       "range", ""},
      {"starting-health", "Starting health",
       "Hearts granted at expedition start.", "range", ""},
      {"ghost-arrival", "Ghost arrival",
       "Seconds before the stage begins hunting.", "range", ""},
      {"shop-frequency", "Shop frequency", "Merchant room availability.",
       "select", ""},
      {"shortcuts", "Discovered shortcuts", "Allow unlocked route entrances.",
       "toggle", ""},
      {"treasury", "Shared treasury", "Combine party currency.", "toggle", ""},
      {"friendly-fire", "Friendly fire", "Players can damage each other.",
       "toggle", ""},
      {"lantern-fuel", "Lantern fuel",
       "Duration of carried expedition lanterns.", "select", "Old Lanterns"}};
  std::ostringstream out;
  out << R"(<div class="subview-header panel"><button data-focus data-action="play-lobby" class="button">‹ Back to lobby</button><div><small>SPLONKS SESSION RULES</small><h2>Expedition settings</h2><p>10 settings · 1 contributed by active mods.</p></div><button data-focus data-action="reset-rules" class="button">Reset mode defaults</button></div><div class="master-detail rules"><section class="panel master-list scroll-list"><div class="scroll-body">)";
  for (const Rule &rule : rules) {
    out << R"(<div class=")"
        << markup::selected_class(state_.selected_rule == rule.name, "rule-row")
        << R"("><span><strong>)" << rule.name << R"(</strong><small>)"
        << rule.note << R"(</small>)";
    if (*rule.source)
      out << R"(<em>MOD · )" << rule.source << R"(</em>)";
    out << R"(</span><div class="rule-control">)";
    const std::string id = rule.id;
    if (id == "difficulty")
      markup::append_select(out, "rule-value-difficulty",
                    markup::map_value(state_.rule_values, id),
                    {"Relaxed", "Standard", "Dangerous", "Nightmare"},
                    "native-select rule-select");
    else if (id == "variation")
      markup::append_select(out, "rule-value-variation",
                    markup::map_value(state_.rule_values, id),
                    {"Quest-authored", "Remixed", "Fully random"},
                    "native-select rule-select");
    else if (id == "shop-frequency")
      markup::append_select(
          out, "rule-value-shop-frequency", markup::map_value(state_.rule_values, id),
          {"Rare", "Normal", "Frequent"}, "native-select rule-select");
    else if (id == "lantern-fuel")
      markup::append_select(
          out, "rule-value-lantern-fuel", markup::map_value(state_.rule_values, id),
          {"Scarce", "Standard", "Generous"}, "native-select rule-select");
    else if (std::string(rule.kind) == "range") {
      const int minimum = id == "ghost-arrival" ? 30 : 1;
      const int maximum = id == "ghost-arrival" ? 300 : 9;
      const int step = id == "ghost-arrival" ? 10 : 1;
      const std::string value = markup::map_value(state_.rule_values, id);
      out << R"(<input data-focus data-action="rule-value-)" << id
          << R"(" class="native-range rule-range" type="range" min=")"
          << minimum << R"(" max=")" << maximum << R"(" step=")" << step
          << R"(" value=")" << value << R"("/><b id="rule-output-)" << id
          << R"(">)" << value
          << (id == "starting-health" ? " hearts"
              : id == "ghost-arrival" ? " sec"
                                      : "")
          << R"(</b>)";
    } else {
      const bool checked = id == "shortcuts"  ? state_.shortcuts
                           : id == "treasury" ? state_.shared_treasury
                                              : state_.friendly_fire;
      out << R"(<input data-focus data-action="rule-value-)" << id
          << R"(" class="native-toggle" type="checkbox" value="true")";
      if (checked)
        out << R"( checked="checked")";
      out << R"(/><b id="rule-output-)" << id << R"(">)" << markup::on_off(checked)
          << R"(</b>)";
    }
    out << R"(</div></div>)";
  }
  std::string selected_value = "Configured";
  std::string selected_description =
      "This game-provided rule is serialized into the session manifest and "
      "synchronized before launch.";
  if (state_.selected_rule == "Discovered shortcuts") {
    selected_value = markup::on_off(state_.shortcuts);
    selected_description =
        "Lets the party enter from previously discovered quest junctions. "
        "Disable it for a clean route beginning at stage one.";
  } else if (state_.selected_rule == "Shared treasury") {
    selected_value = markup::on_off(state_.shared_treasury);
    selected_description =
        "Combines collected currency for the party instead of tracking a "
        "separate wallet per local or remote player.";
  } else if (state_.selected_rule == "Friendly fire") {
    selected_value = markup::on_off(state_.friendly_fire);
    selected_description =
        "Controls whether attacks and hazards authored as player damage may "
        "affect other members of the party.";
  } else if (state_.selected_rule == "Shared lives")
    selected_value = markup::map_value(state_.rule_values, "shared-lives");
  else if (state_.selected_rule == "Starting health")
    selected_value =
        markup::map_value(state_.rule_values, "starting-health") + " hearts";
  else if (state_.selected_rule == "Ghost arrival")
    selected_value = markup::map_value(state_.rule_values, "ghost-arrival") + " sec";
  else if (state_.selected_rule == "Expedition difficulty")
    selected_value = markup::map_value(state_.rule_values, "difficulty");
  else if (state_.selected_rule == "Stage variation")
    selected_value = markup::map_value(state_.rule_values, "variation");
  else if (state_.selected_rule == "Shop frequency")
    selected_value = markup::map_value(state_.rule_values, "shop-frequency");
  else if (state_.selected_rule == "Lantern fuel")
    selected_value =
        markup::map_value(state_.rule_values, "lantern-fuel") + " · Old Lanterns";
  out << R"(</div></section><aside class="panel detail rule-detail"><button data-focus data-action="play-lobby" class="mobile-back">‹ All rules</button><small>SELECTED RULE</small><h2 id="rule-detail-title">)"
      << state_.selected_rule << R"(</h2><p>)" << selected_description
      << R"(</p><div class="value-box"><small>CURRENT VALUE</small><strong id="rule-detail-current">)"
      << selected_value
      << R"(</strong></div><div class="impact"><h3>SESSION EFFECT</h3><div class="kv"><span>Activity</span><b>Continue expedition</b></div><div class="kv"><span>Applies to</span><b>Future stages</b></div><div class="kv"><span>Authority</span><b>Lobby host</b></div><div class="kv"><span>Synced</span><b>Before launch</b></div></div><p class="caption">The game supplies rule definitions. Gubsy supplies stable editor widgets, focus, serialization, and synchronization.</p></aside></div>)";
  return out.str();
}

std::string GubsyApp::BuildSessionMods() const {
  struct Mod {
    const char *name;
    const char *version;
    const char *note;
    bool active;
  };
  constexpr Mod mods[]{
      {"Base Content", "1.4.0", "Required root package", true},
      {"Old Lanterns", "1.3.1", "Needs update to 1.4.0", false},
      {"Underground Rivers", "2.2.0", "1 required dependency", true},
      {"Mycelium Below", "1.1.0", "Hot-safe runtime hooks", true},
      {"Brassline Grapple Kit", "0.8.4", "Applies next stage", true},
      {"Echoing Markets", "1.0.2", "2 required dependencies", true},
      {"Temple Weather", "3.0.0", "New-session recommended", true}};
  std::ostringstream out;
  const bool continuing = state_.activity == "Continue expedition";
  out << R"(<div class="subview-header panel session-mod-head"><button data-focus data-action="play-lobby" class="button">‹ Back to lobby</button><div><small>CURRENT LOBBY / CONTENT</small><h2>Session mods</h2><p>)"
      << (continuing
              ? "Checkpoint \"Temple safe room\" has a recorded package set."
              : "Choose content for this new session before launch.")
      << R"(</p></div><div class="local-tabs"><button data-focus data-action="session-current" class=")"
      << markup::selected_class(!state_.session_mod_browse)
      << R"(">Current set</button><button data-focus data-action="session-browse" class=")"
      << markup::selected_class(state_.session_mod_browse)
      << R"(">Browse &amp; add</button></div></div>)";
  if (state_.session_mod_browse) {
    struct Candidate {
      const char *name;
      const char *author;
      const char *note;
      const char *dependency;
    };
    constexpr Candidate candidates[]{
        {"Mycelium Below", "Root & Branch", "Worlds · Biomes · Co-op",
         "Underground Rivers will install"},
        {"Brassline Grapple Kit", "Clockwork Moss", "Mechanics · Traversal",
         "Cartographer's Desk installed"},
        {"Skybreak Caverns", "Gale Assembly", "Worlds · Sky",
         "No missing dependencies"},
        {"Ember Orchard", "Kindling Club", "Biome · Items",
         "Base Content installed"},
        {"Clockwork Descent", "Ratchet House", "Quest · Machines",
         "No missing dependencies"},
        {"Friendly Ghosts", "Pale Picnic", "Creatures · Co-op",
         "Base Content installed"},
        {"Campfire Stories", "Wayfarer", "Quests · Dialogue",
         "Old Lanterns update required"},
        {"Relay Races", "Split Second", "Mode · Co-op",
         "Networking 1.2 installed"}};
    out << R"(<div class="catalog-tools"><input data-focus data-action="search-catalog" class="search-field" type="text" value=")"
        << markup::escape_attribute(state_.mod_filter)
        << R"(" placeholder="Search compatible session content…"/><label class="check-control"><input data-focus data-action="compatible-value" class="native-toggle" type="checkbox" value="true")";
    if (state_.compatible_only)
      out << R"( checked="checked")";
    out << R"(/><span>Compatible only<small>Resolve against this lobby</small></span></label></div><div class="master-detail"><section class="panel master-list scroll-list catalog-list"><div class="scroll-body"><small>AVAILABLE FOR THIS SESSION</small>)";
    for (const Candidate &candidate : candidates) {
      if (!markup::contains_ci(candidate.name, state_.mod_filter) &&
          !markup::contains_ci(candidate.author, state_.mod_filter) &&
          !markup::contains_ci(candidate.note, state_.mod_filter))
        continue;
      const size_t image_index = static_cast<size_t>(&candidate - candidates);
      out << R"(<button data-focus data-action="session-mod-)" << candidate.name
          << R"(" class=")"
          << markup::selected_class(state_.selected_mod == candidate.name,
                            "catalog-row")
          << R"("><img class="mod-thumb" src="../mods/)"
          << markup::mod_image(image_index) << R"("/><span><strong>)" << candidate.name
          << R"(</strong><small>)" << candidate.note << " · by "
          << candidate.author << R"(</small><em>)" << candidate.dependency
          << R"(</em></span><b>SESSION READY</b><i></i></button>)";
    }
    out << R"(</div></section><aside class="panel detail mod-detail"><img class="hero-art" src="../mods/sheet-1-0.png"/><small>ADD TO CURRENT SESSION</small><h2>)"
        << state_.selected_mod
        << R"(</h2><p>Review the complete solved change plan here; installation and activation are one operation from the lobby.</p><div class="impact"><h3>SESSION CHANGE PLAN</h3><div class="kv"><span>Package</span><b>Install latest compatible</b></div><div class="kv"><span>Dependencies</span><b>1 automatic</b></div><div class="kv"><span>Activation</span><b>This lobby</b></div><div class="kv"><span>Runtime policy</span><b>Safe before launch</b></div></div><div class="dependency"><h3>AUTOMATIC DEPENDENCIES</h3><div><span>↳ Base Content v1.4.0</span><b>INSTALLED</b></div><div><span>↳ Underground Rivers v2.2.0</span><b>WILL INSTALL</b></div></div><button data-focus data-action="install-add-session" class="button primary">Install dependencies &amp; add to session</button><button data-focus data-action="session-current" class="button">Cancel</button></aside></div>)";
    return out.str();
  }
  if (continuing)
    out << R"(<div class="manifest-banner"><span><strong>Checkpoint mod set differs from this lobby</strong><small>1 missing · 1 inactive · 2 extra</small></span><button data-focus data-action="sync-manifest" class="button primary">Install &amp; sync checkpoint set</button></div>)";
  else
    out << R"(<div class="manifest-banner clean"><span><strong>New-session manifest</strong><small>Changes here become the recorded starting set.</small></span><b>7 ACTIVE PACKAGES</b></div>)";
  out << R"(<div class="master-detail"><section class="panel master-list scroll-list"><div class="scroll-body">)";
  for (const Mod &mod : mods) {
    const size_t image_index = static_cast<size_t>(&mod - mods);
    out << R"(<button data-focus data-action="session-mod-)" << mod.name
        << R"(" class=")"
        << markup::selected_class(state_.selected_mod == mod.name, "mod-row")
        << R"("><img class="mod-thumb" src="../mods/)" << markup::mod_image(image_index)
        << R"("/><span><strong>)" << mod.name << R"(</strong><small>v)"
        << mod.version << " · " << mod.note << R"(</small></span><b class=")"
        << (mod.active ? "active" : "") << R"(">)"
        << (mod.active ? "ACTIVE" : "UPDATE → 1.4.0") << R"(</b></button>)";
  }
  out << R"(</div></section><aside class="panel detail mod-detail"><button data-focus data-action="play-lobby" class="mobile-back">‹ All session mods</button><div class="hero-art"></div><small>SESSION CONTENT</small><h2>)"
      << state_.selected_mod
      << R"(</h2><p>Review this package's version, runtime policy, dependencies, and dependents without leaving the session.</p>)";
  if (state_.selected_mod == "Old Lanterns")
    out << R"(<div class="warning-box"><strong>Update required before use</strong><small>Installed v1.3.1 → compatible v1.4.0</small><button data-focus data-action="update-enable" class="button primary">Update &amp; enable</button></div>)";
  else
    out << R"(<div class="value-box"><small>CURRENT SESSION STATE</small><strong>ACTIVE · COMPATIBLE</strong></div>)";
  out << R"(<div class="dependency"><h3>Required dependencies</h3><div><span>↳ Base Content</span><b>INSTALLED</b></div><h3>Required by installed mods</h3><div><span>↑ Temple Weather</span><b>ACTIVE</b></div></div></aside></div>)";
  return out.str();
}
