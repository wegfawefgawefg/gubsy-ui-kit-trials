#include "app.h"
#include "ui_markup.h"

#include <iomanip>
#include <sstream>

std::string GubsyApp::BuildControls() const {
  // build control section tabs
  std::ostringstream out;
  const char *tabs[]{"Bindings", "Devices", "Input tuning"};
  out << R"(<div class="local-tabs">)";
  for (const char *tab : tabs)
    out << R"(<button data-focus data-action="controls-tab-)" << tab
        << R"(" class=")" << markup::selected_class(state_.controls_tab == tab)
        << R"(">)" << tab << R"(</button>)";
  out << R"(<span>LB / RB change section</span></div>)";
  if (state_.controls_tab == "Bindings") {
    // define game action schema
    struct Action {
      const char *name;
      const char *type;
      const char *summary;
    };
    constexpr Action actions[]{
        {"Menu Up", "Digital action · Menu", "D-Pad Up"},
        {"Menu Down", "Digital action · Menu", "D-Pad Down"},
        {"Menu Left", "Digital action · Menu", "D-Pad Left"},
        {"Menu Right", "Digital action · Menu", "D-Pad Right"},
        {"Activate", "Digital action · Menu", "Gamepad A"},
        {"Cancel", "Digital action · Menu", "Gamepad B"},
        {"Move", "Vector2 action · Player", "Left Stick"},
        {"Look", "Vector2 action · Player", "Right Stick"},
        {"Accelerate", "Scalar action · Vehicle", "Right Trigger"},
        {"Brake", "Scalar action · Vehicle", "Left Trigger"}};
    // build binding action list and editor
    out << R"(<div class="toolbar"><input data-focus data-action="filter-actions" class="search-field" type="text" value=")"
        << markup::escape_attribute(state_.control_filter)
        << R"(" placeholder="Filter actions…"/>)";
    markup::append_select(out, "choice-value-bind-profile",
                  markup::map_value(state_.choice_values, "bind-profile"),
                  {"Default Binds", "Arcade Binds", "Vehicle Binds"});
    out << R"(<button data-focus data-action="new-binds" class="button">+ New</button><button data-focus data-action="rename-binds" class="button">Rename</button><button data-focus data-action="reset-binds" class="button">Reset</button><button data-focus data-action="delete-binds" class="button danger">Delete</button></div><div class="master-detail"><section class="panel master-list scroll-list"><div class="scroll-body">)";
    for (const Action &action : actions) {
      if (!markup::contains_ci(action.name, state_.control_filter) &&
          !markup::contains_ci(action.type, state_.control_filter) &&
          !markup::contains_ci(action.summary, state_.control_filter))
        continue;
      out << R"(<button data-focus data-action="action-)" << action.name
          << R"(" class=")"
          << markup::selected_class(state_.selected_action == action.name, "action-row")
          << R"("><span><strong>)" << action.name << R"(</strong><small>)"
          << action.type << R"(</small></span><b>)" << action.summary
          << R"(</b></button>)";
    }
    out << R"(</div>)";
    out << R"(</section><aside class="panel detail binding-detail"><small>SELECTED LOGICAL ACTION</small><h2>)"
        << state_.selected_action
        << R"(</h2><p>Logical actions accept multiple physical controls. A binding may listen for input or explicitly browse a device, control, direction, conversion, and qualifier.</p><div class="binding-row"><em>1</em><span><strong>D-Pad Up</strong><small>Xbox Wireless Controller · digital</small></span><button data-focus data-action="capture-binding" class="text-button">Listen</button><button data-focus data-action="choose-binding" class="text-button">Choose</button><button data-focus data-action="remove-binding" class="remove">×</button></div><div class="binding-row"><em>2</em><span><strong>Keyboard W</strong><small>Keyboard + Mouse · digital</small></span><button data-focus data-action="capture-binding" class="text-button">Listen</button><button data-focus data-action="choose-binding" class="text-button">Choose</button><button data-focus data-action="remove-binding" class="remove">×</button></div><div class="binding-row"><em>3</em><span><strong>Right Trigger → Button</strong><small>Threshold 0.62 · rising edge · explicit conversion</small></span><button data-focus data-action="capture-binding" class="text-button">Listen</button><button data-focus data-action="choose-binding" class="text-button">Choose</button><button data-focus data-action="remove-binding" class="remove">×</button></div><div class="actions"><button data-focus data-action="capture-binding" class="button primary">)"
        << (state_.capture_mode ? "Listening…" : "+ Listen for input")
        << R"(</button><button data-focus data-action="choose-binding" class="button">Browse controls…</button></div><div class="conversion-panel"><h3>EXPLICIT BINDING EDITOR</h3><div class="form-row"><span>Device<small>Any, owned device, or exact instance</small></span>)";
    markup::append_select(out, "choice-value-binding-device",
                  markup::map_value(state_.choice_values, "binding-device"),
                  {"Any owned device", "Xbox Wireless Controller",
                   "T-LCM Pedals", "T.16000M Flight Stick"});
    out << R"(</div><div class="form-row"><span>Physical control<small>Axes, hats, buttons, keys, gestures</small></span>)";
    markup::append_select(out, "choice-value-binding-control",
                  markup::map_value(state_.choice_values, "binding-control"),
                  {"Right Trigger", "Left Trigger", "Button South", "Axis 0"});
    out << R"(</div><div class="form-row"><span>Conversion<small>Scalar input drives a digital action</small></span>)";
    markup::append_select(out, "choice-value-binding-conversion",
                  markup::map_value(state_.choice_values, "binding-conversion"),
                  {"Axis → Button", "Axis → Scalar", "Button → Scalar",
                   "Raw passthrough"});
    out << R"(</div><div class="form-row"><span>Threshold / edge<small>Qualify activation without losing the source</small></span>)";
    markup::append_select(
        out, "choice-value-binding-threshold",
        markup::map_value(state_.choice_values, "binding-threshold"),
        {"0.25 · Rising", "0.50 · Rising", "0.62 · Rising", "0.75 · Falling"});
    out << R"(</div></div></aside></div>)";
  } else if (state_.controls_tab == "Devices") {
    // define recognized device records
    struct Device {
      const char *action;
      const char *name;
      const char *info;
      const char *owner;
    };
    constexpr Device devices[]{
        {"device-keyboard", "Keyboard + Mouse", "Keyboard / pointer · Built in",
         "Moss"},
        {"device-xbox", "Xbox Wireless Controller", "Gamepad · Bluetooth · 80%",
         "Moss"},
        {"device-dualsense", "DualSense Wireless Controller", "Gamepad · USB",
         "UNASSIGNED"},
        {"device-flight", "T.16000M Flight Stick",
         "Joystick · USB · 16 buttons", "UNASSIGNED"},
        {"device-pedals", "T-LCM Pedals", "Pedals · USB · 3 axes", "Moss"}};
    // build device ownership and raw input views
    out << R"(<div class="toolbar"><span class="toolbar-title"><small>RECOGNIZED DEVICES</small>5 connected · one player may own many</span><button data-focus data-action="refresh-devices" class="button">↻ Rescan</button></div><div class="master-detail"><section class="panel master-list scroll-list"><div class="scroll-body">)";
    for (const Device &device : devices)
      out << R"(<button data-focus data-action=")" << device.action
          << R"(" class=")"
          << markup::selected_class(state_.selected_device == device.name, "device-row")
          << R"("><span><strong>)" << device.name << R"(</strong><small>)"
          << device.info << R"(</small></span><b>)" << device.owner
          << R"(</b></button>)";
    out << R"(</div>)";
    out << R"(</section><aside class="panel detail device-detail"><small>RECOGNIZED INPUT DEVICE</small><h2>)"
        << state_.selected_device
        << R"(</h2><p>Assign this device to one local player. A player can combine a keyboard, wheel, pedals, flight stick, macro pad, and gamepad.</p><div class="owner-list"><button data-focus data-action="unassign" class="owner"><span>○</span><b>Unassigned</b><small>Ignore for local gameplay</small></button><button data-focus data-action="assign-moss" class="owner selected"><span>P1</span><b>Moss</b><small>Uses Default Binds</small></button><button data-focus data-action="assign-vega" class="owner"><span>P2</span><b>Vega</b><small>Uses Arcade Binds</small></button></div><div class="explorer"><div class="section-title"><span><small>LIVE INPUT EXPLORER</small><strong>Actuate anything on the device</strong></span><button data-focus data-action="identify-device" class="button">Identify</button></div><div class="raw-control"><span>)"
        << state_.raw_input_name << R"(</span><b>)" << std::fixed
        << std::setprecision(3) << state_.raw_input_value
        << R"(</b><em><i style="width:)"
        << static_cast<int>(state_.raw_input_value * 100.0f)
        << R"(%;"></i></em></div><div class="raw-control"><span>Button 1 · South</span><b>UP</b><em><i style="width:0%;"></i></em></div><div class="raw-control"><span>Hat 0 · D-Pad</span><b>UP-RIGHT</b><em><i style="width:66%;"></i></em></div><p>This raw view reveals names and ranges exactly as Gubsy recognizes unusual hardware.</p></div></aside></div>)";
  } else {
    // build device tuning controls
    out << R"(<div class="master-detail tuning-workspace"><section class="panel master-list scroll-list"><div class="scroll-body"><div class="section-title"><span><small>INPUT PROFILE</small><strong>Standard</strong></span><b>Xbox Wireless Controller</b></div>)";
    auto tuning_range = [&](const char *id, const char *name,
                            const char *note) {
      const std::string value = markup::map_value(state_.tuning_values, id);
      out << R"(<div class="tuning-row"><span><strong>)" << name
          << R"(</strong><small>)" << note
          << R"(</small></span><input data-focus data-action="tuning-value-)"
          << id
          << R"(" class="native-range tuning-range" type="range" min="0" max="100" step="1" value=")"
          << value << R"("/><b id="tuning-output-)" << id << R"(">)" << value
          << R"(%</b></div>)";
    };
    tuning_range("look-sensitivity", "Look sensitivity",
                 "Horizontal and vertical camera speed");
    tuning_range("stick-deadzone", "Stick deadzone",
                 "Ignore small stick movement near center");
    tuning_range("vibration", "Vibration strength", "Controller rumble output");
    tuning_range("trigger-deadzone", "Trigger deadzone",
                 "Minimum pedal or trigger travel");
    out << R"(<div class="form-row"><span>Response curve<small>Stick magnitude transformation</small></span>)";
    markup::append_select(out, "tuning-value-response-curve",
                  markup::map_value(state_.tuning_values, "response-curve"),
                  {"Linear", "Smooth", "Aggressive", "Custom"});
    out << R"(</div><div class="form-row"><span>Invert Y-axis<small>Reverse vertical camera movement</small></span><div class="toggle-with-label"><input data-focus data-action="tuning-value-invert-y" class="native-toggle" type="checkbox" value="true")";
    if (markup::map_value(state_.tuning_values, "invert-y") == "true")
      out << R"( checked="checked")";
    out << R"(/><b id="tuning-output-invert-y">)"
        << (markup::map_value(state_.tuning_values, "invert-y") == "true" ? "ON"
                                                                  : "OFF")
        << R"(</b></div></div><button data-focus data-action="reset-tuning" class="button">Reset input profile</button></div></section><aside class="panel detail response-preview"><small>DEVICE RESPONSE</small><h2>Live response</h2><div class="response-circle"><i></i></div><dl class="summary"><dt>Device</dt><dd>Xbox Wireless Controller</dd><dt>Deadzone</dt><dd id="tuning-summary-stick-deadzone">)"
        << markup::map_value(state_.tuning_values, "stick-deadzone")
        << R"(%</dd><dt>Vibration</dt><dd id="tuning-summary-vibration">)"
        << markup::map_value(state_.tuning_values, "vibration")
        << R"(%</dd><dt>Curve</dt><dd id="tuning-summary-response-curve">)"
        << markup::map_value(state_.tuning_values, "response-curve")
        << R"(</dd></dl><p>Move the device to compare raw and qualified output.</p></aside></div>)";
  }
  return out.str();
}
