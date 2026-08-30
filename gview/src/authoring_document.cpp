#include "authoring_document.hpp"

#include <algorithm>
#include <cmath>

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

// What is: Generated metrics copied into legacy nodes while authored child
// order and newly created children survive.
void refresh_layout_tree(glayout::GraphNode& authored, const glayout::GraphNode& generated) {
    authored.label = generated.label;
    authored.measure_key = generated.measure_key;
    authored.container = generated.container;
    authored.size = generated.size;
    authored.padding = generated.padding;
    authored.gap = generated.gap;
    authored.columns = generated.columns;
    authored.align = generated.align;
    authored.distribution = generated.distribution;
    authored.clip = generated.clip;
    authored.visible = generated.visible;
    authored.mask = generated.mask;
    authored.anchors = generated.anchors;
    for (const glayout::GraphNode& generated_child : generated.children) {
        const auto found = std::find_if(
            authored.children.begin(), authored.children.end(),
            [&](const glayout::GraphNode& child) { return child.id == generated_child.id; });
        if (found != authored.children.end()) refresh_layout_tree(*found, generated_child);
    }
}

// What is: Recursive removal for a retired generated element.
bool remove_layout_node(glayout::GraphNode& parent, std::string_view id) {
    const auto found = std::find_if(parent.children.begin(), parent.children.end(),
                                    [&](const auto& child) { return child.id == id; });
    if (found != parent.children.end()) {
        parent.children.erase(found);
        return true;
    }
    for (glayout::GraphNode& child : parent.children)
        if (remove_layout_node(child, id)) return true;
    return false;
}

// What is: Host-owned values refreshed without replacing authored presentation.
void refresh_live_content(gview::NodeSpec& authored, const gview::NodeSpec& generated) {
    authored.text = generated.text;
    authored.asset = generated.asset;
    authored.selected = generated.selected;
    authored.enabled = generated.enabled;
    authored.options = generated.options;
    authored.condition = generated.condition;
    authored.text_style.vertical = generated.text_style.vertical;
}

// What is: One-time adoption of the balanced workspace gutter. The narrow
// signature keeps later explicit authoring edits intact.
void migrate_legacy_workspace_padding(gview::View& authored, const gview::View& generated) {
    glayout::GraphNode* target = glayout::find_graph_node(authored.layout, "main");
    const glayout::GraphNode* source = glayout::find_graph_node(generated.layout, "main");
    if (!target || !source) return;

    const float legacy_side = source->padding.left * 2.0f;
    const bool legacy_default = std::abs(target->padding.left - legacy_side) < 0.1f &&
                                std::abs(target->padding.right - legacy_side) < 0.1f &&
                                target->padding.bottom < source->padding.bottom;
    if (!legacy_default) return;
    target->padding.left = source->padding.left;
    target->padding.right = source->padding.right;
    target->padding.bottom = source->padding.bottom;
}

} // namespace

// Migrates the pre-compact-shell documents once, using the removed breadcrumb
// as a schema marker.
void migrate_authored_view(gview::View& authored, const gview::View& generated) {
    migrate_legacy_workspace_padding(authored, generated);

    // What is: The shell content node became structural; remove its legacy
    // presentation spec while preserving the authored layout container.
    const bool generated_content_spec =
        std::any_of(generated.nodes.begin(), generated.nodes.end(), [](const auto& node) {
            return node.layout_id == "content";
        });
    const bool retired_content_spec =
        !generated_content_spec &&
        std::any_of(authored.nodes.begin(), authored.nodes.end(), [](const auto& node) {
            return node.layout_id == "content";
        });
    if (!generated_content_spec)
        std::erase_if(authored.nodes,
                      [](const auto& node) { return node.layout_id == "content"; });

    // What is: The same shell migration adopts minimum safe-area gutters for
    // existing authored scroll hosts without replacing larger user padding.
    if (retired_content_spec) {
        for (const gview::NodeSpec& spec : generated.nodes) {
            if (spec.control != gview::ControlKind::ScrollArea) continue;
            const glayout::GraphNode* source =
                glayout::find_graph_node(generated.layout, spec.layout_id);
            glayout::GraphNode* target =
                glayout::find_graph_node(authored.layout, spec.layout_id);
            if (!source || !target) continue;
            target->clip = source->clip;
            target->padding.left = std::max(target->padding.left, source->padding.left);
            target->padding.top = std::max(target->padding.top, source->padding.top);
            target->padding.right = std::max(target->padding.right, source->padding.right);
            target->padding.bottom = std::max(target->padding.bottom, source->padding.bottom);
        }
    }

    if (!glayout::find_graph_node(authored.layout, "breadcrumb") ||
        glayout::find_graph_node(generated.layout, "breadcrumb"))
        return;
    refresh_layout_tree(authored.layout.root, generated.layout.root);
    remove_layout_node(authored.layout.root, "breadcrumb");
    std::erase_if(authored.nodes,
                  [](const gview::NodeSpec& node) { return node.layout_id == "breadcrumb"; });
}

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
