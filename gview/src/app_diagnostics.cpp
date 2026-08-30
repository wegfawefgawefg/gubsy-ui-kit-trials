#include "app.hpp"

// What is: Read-only focus and model evidence used by trial diagnostics.
std::string TrialApp::focus_id() const {
    const gview::NodeIndex focus = runtime_.focus();
    if (focus == gview::invalid_node || focus >= runtime_.view().nodes.size()) return {};
    return runtime_.view().nodes[focus].source.layout_id;
}

bool TrialApp::node_selected(std::string_view id) const {
    const auto found = runtime_.view().indices.find(std::string(id));
    return found != runtime_.view().indices.end() &&
           runtime_.view().nodes[found->second].source.selected;
}

gview::Value TrialApp::value(std::string_view key) const { return model_.read(key); }

bool TrialApp::focus_open() const {
    const gview::NodeIndex focus = runtime_.focus();
    return focus != gview::invalid_node && focus < runtime_.state().size() &&
           runtime_.state()[focus].open;
}

float TrialApp::scroll_offset(std::string_view id) const {
    const auto found = runtime_.view().indices.find(std::string(id));
    return found == runtime_.view().indices.end() ? 0.0f : runtime_.state()[found->second].scroll;
}

// What is: Authoring visibility and timing evidence kept outside app flow.
bool TrialApp::authoring_enabled() const { return authoring_enabled_; }
void TrialApp::set_authoring_enabled(bool enabled, bool restore_preferences) {
    if (enabled && restore_preferences) restore_authoring_preferences();
    else if (enabled) authoring_preferences_restored_ = true;
    authoring_enabled_ = enabled;
}

double TrialApp::update_ms() const { return update_ms_; }
double TrialApp::render_ms() const { return render_ms_; }
double TrialApp::compile_ms() const { return compile_ms_; }
double TrialApp::activation_ms() const { return activation_ms_; }
const gview::RuntimeStats& TrialApp::stats() const { return runtime_.stats(); }
std::size_t TrialApp::owned_bytes() const { return runtime_.owned_bytes(); }
