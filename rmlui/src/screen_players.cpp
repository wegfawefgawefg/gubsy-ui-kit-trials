#include "app.h"
#include "ui_markup.h"

#include <sstream>

// Local roster, profile, and device ownership surfaces.

std::string GubsyApp::BuildPlayers() const {
  std::ostringstream out;
  out << R"(<div class="local-tabs"><button data-focus data-action="player-tab-Local players" class=")"
      << markup::selected_class(state_.player_tab == "Local players")
      << R"(">Local players</button><button data-focus data-action="player-tab-Profiles" class=")"
      << markup::selected_class(state_.player_tab == "Profiles")
      << R"(">Profiles</button><button data-focus data-action="player-tab-Devices" class=")"
      << markup::selected_class(state_.player_tab == "Devices")
      << R"(">Devices</button><span>LB / RB change section</span></div>)";
  if (state_.player_tab == "Local players") {
    out << R"(<div class="master-detail"><section class="panel master-list"><div class="section-title"><span><small>LOCAL ROSTER</small><strong>2 / 4 players</strong></span><button data-focus data-action="add-player" class="button primary">+</button></div><button data-focus data-action="profile-moss" class="list-row selected"><em>P1</em><span><strong>Moss</strong><small>Xbox Wireless Controller</small></span><b>)"
        << (state_.player_ready ? "READY" : "NOT READY")
        << R"(</b></button><button data-focus data-action="add-player" class="list-row"><em>P2</em><span><strong>Open slot</strong><small>Invite or join locally</small></span><b>OPEN</b></button></section><aside class="panel detail"><small>PLAYER 1 DETAILS</small><h2>Moss</h2><div class="form-row"><span>Player profile<small>Persistent identity and history</small></span>)";
    markup::append_select(out, "choice-value-player-profile",
                  markup::map_value(state_.choice_values, "player-profile"),
                  {"Moss", "Vega", "Guest"});
    out << R"(</div><div class="form-row"><span>Bindings<small>Independent action map</small></span>)";
    markup::append_select(out, "choice-value-player-bindings",
                  markup::map_value(state_.choice_values, "player-bindings"),
                  {"Default Binds", "Arcade Binds", "Vehicle Binds"});
    out << R"(</div><div class="form-row"><span>Input tuning<small>Deadzone and response profile</small></span>)";
    markup::append_select(out, "choice-value-player-tuning",
                  markup::map_value(state_.choice_values, "player-tuning"),
                  {"Standard", "Precise", "Accessible", "Custom"});
    out << R"(</div><h3>ASSIGNED DEVICES · MANY ALLOWED</h3><div class="device-chip"><span>Xbox Wireless Controller<small>Gamepad · connected</small></span><button data-focus data-action="unassign" class="button">Remove</button></div><div class="device-chip"><span>T-LCM Pedals<small>Pedal axis set · optional</small></span><button data-focus data-action="unassign" class="button">Remove</button></div><div class="actions"><button data-focus data-action="assign-device" class="button">+ Assign device</button><button data-focus data-action="toggle-ready" class="button primary">)"
        << (state_.player_ready ? "Mark not ready" : "Mark ready")
        << R"(</button></div></aside></div>)";
  } else if (state_.player_tab == "Profiles") {
    struct Profile {
      const char *action;
      const char *initials;
      const char *name;
      const char *stats;
    };
    constexpr Profile profiles[]{
        {"profile-moss", "MO", "Moss", "38h 22m · 84 runs · 21 wins"},
        {"profile-vega", "VE", "Vega", "14h 11m · 31 runs · 4 wins"},
        {"profile-guest", "GU", "Guest", "Never · 0 runs · 0 wins"}};
    out << R"(<section class="panel profile-workspace"><div class="section-title"><span><small>PROFILE LIBRARY</small><strong>3 profiles</strong></span><button data-focus data-action="new-profile" class="button primary">+ New profile</button></div><div class="profile-cards">)";
    for (const Profile &profile : profiles)
      out << R"(<button data-focus data-action=")" << profile.action
          << R"(" class=")"
          << markup::selected_class(state_.selected_profile == profile.name,
                            "profile-card")
          << R"("><b>)" << profile.initials << R"(</b><strong>)" << profile.name
          << R"(</strong><small>)" << profile.stats << R"(</small><em>)"
          << (state_.selected_profile == profile.name ? "ACTIVE" : "SELECT")
          << R"(</em></button>)";
    out << R"(</div><div class="profile-history"><small>PROFILE DATA IS NOT A SAVE</small><h2>)"
        << state_.selected_profile
        << R"('s history</h2><div class="stat-strip"><span><b>84</b>PLAYS</span><span><b>21</b>WINS</span><span><b>138</b>DEATHS</span><span><b>2.4M</b>SCORE</span><span><b>42</b>REPLAYS</span></div><p>Achievements, aggregate stats, unlocks, scores, and replays survive across campaigns and checkpoints.</p></div></section>)";
  } else {
    struct Device {
      const char *action;
      const char *name;
      const char *owner;
      const char *kind;
    };
    constexpr Device devices[]{
        {"device-xbox", "Xbox Wireless Controller", "Moss",
         "Gamepad · battery 82%"},
        {"device-keyboard", "Keyboard + Mouse", "Unassigned",
         "Keyboard / pointer"},
        {"device-dualsense", "DualSense Wireless Controller", "Unassigned",
         "Gamepad · USB"},
        {"device-flight", "T.16000M Flight Stick", "Unassigned",
         "Joystick · 16 buttons · 4 axes"},
        {"device-pedals", "T-LCM Pedals", "Moss", "Pedals · 3 axes"}};
    out << R"(<div class="master-detail"><section class="panel master-list scroll-list"><div class="scroll-body"><div class="section-title"><span><small>RECOGNIZED DEVICES</small><strong>5 connected</strong></span><button data-focus data-action="refresh-devices" class="button">↻ Refresh</button></div>)";
    for (const Device &device : devices)
      out << R"(<button data-focus data-action=")" << device.action
          << R"(" class=")"
          << markup::selected_class(state_.selected_device == device.name, "device-row")
          << R"("><span><strong>)" << device.name << R"(</strong><small>)"
          << device.kind << R"(</small></span><b>)" << device.owner
          << R"(</b></button>)";
    out << R"(</div>)";
    out << R"(</section><aside class="panel detail"><small>DEVICE OWNERSHIP</small><h2>)"
        << state_.selected_device
        << R"(</h2><p>A device may be assigned to one local player; each player may own any number of devices.</p><div class="owner-list"><button data-focus data-action="unassign" class="owner"><span>○</span><b>Unassigned</b><small>Ignore this device for local gameplay</small></button><button data-focus data-action="assign-moss" class="owner selected"><span>P1</span><b>Moss</b><small>Uses Default Binds</small></button><button data-focus data-action="assign-vega" class="owner"><span>P2</span><b>Vega</b><small>Uses Arcade Binds</small></button></div><button data-focus data-action="identify-device" class="button">Identify / rumble device</button></aside></div>)";
  }
  return out.str();
}
