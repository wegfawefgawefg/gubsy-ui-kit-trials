#include "authoring_document.hpp"

#include <algorithm>

namespace {

// What is: Missing generated children added without disturbing authored order.
void merge_layout_children(glayout::GraphNode& authored, const glayout::GraphNode& generated) {
    for (const glayout::GraphNode& generated_child : generated.children) {
        const auto found = std::find_if(
            authored.children.begin(), authored.children.end(),
            [&](const glayout::GraphNode& child) { return child.id == generated_child.id; });
        if (found == authored.children.end()) {
            authored.children.push_back(generated_child);
            continue;
        }
        merge_layout_children(*found, generated_child);
    }
}

// What is: Host-owned values refreshed without replacing authored presentation.
void refresh_live_content(gview::NodeSpec& authored, const gview::NodeSpec& generated) {
    authored.text = generated.text;
    authored.asset = generated.asset;
    authored.selected = generated.selected;
    authored.enabled = generated.enabled;
    authored.options = generated.options;
    authored.condition = generated.condition;
}

} // namespace

// What is: Screen identity independent of transient selection and
// notifications.
std::string authoring_context(const TrialModel& model) {
    if (model.game_ui) return "inventory";
    switch (model.destination) {
    case Destination::Play:
        return "play-" + std::to_string(static_cast<int>(model.play_page));
    case Destination::Players:
        return "players-" + model.players_tab;
    case Destination::Settings:
        return "settings-" + model.settings_tab;
    case Destination::Controls:
        return "controls-" + model.controls_tab;
    case Destination::Progress:
        return "progress";
    case Destination::Mods:
        return "mods-" + model.mods_tab;
    }
    return "screen";
}

// What is: Authored layout/theme retained while generated semantic data stays
// live.
gview::View merge_authored_view(const gview::View& authored, const gview::View& generated) {
    gview::View result = authored;
    result.id = generated.id;
    result.label = generated.label;
    result.layout.id = generated.layout.id;
    result.layout.width = generated.layout.width;
    result.layout.height = generated.layout.height;
    result.layout.dpi_scale = generated.layout.dpi_scale;
    result.layout.form_factor = generated.layout.form_factor;
    merge_layout_children(result.layout.root, generated.layout.root);

    for (const gview::NodeSpec& generated_node : generated.nodes) {
        const auto found = std::find_if(result.nodes.begin(), result.nodes.end(),
                                        [&](const gview::NodeSpec& node) {
                                            return node.layout_id == generated_node.layout_id;
                                        });
        if (found == result.nodes.end()) result.nodes.push_back(generated_node);
        else refresh_live_content(*found, generated_node);
    }
    return result;
}
