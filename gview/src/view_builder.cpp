#include "view_builder.hpp"

#include <algorithm>

namespace {

gview::BoxStyle box(gview::Color fill, gview::Color border, gview::Color text) {
    gview::BoxStyle style;
    style.fill = fill;
    style.border = border;
    style.text = text;
    style.border_width = 1.0f;
    return style;
}

} // namespace

// Builds one resolution-aware view through public GLayout/GView authoring
// structs.
ViewBuilder::ViewBuilder(std::string id, int width, int height) {
    width_ = width;
    scale_ = std::clamp(static_cast<float>(height) / 720.0f, 1.0f, 1.5f);
    view_.id = std::move(id);
    view_.label = view_.id;
    view_.layout.id = view_.id + "_layout";
    view_.layout.width = width;
    view_.layout.height = height;
    view_.layout.root.id = "root";
    view_.layout.root.container = glayout::ContainerKind::Stack;
}

float ViewBuilder::scale() const { return scale_; }
bool ViewBuilder::compact() const { return width_ < 1000 || view_.layout.height < 600; }
bool ViewBuilder::phone() const { return width_ < 600; }
glayout::ContainerKind ViewBuilder::split() const {
    return phone() ? glayout::ContainerKind::Column : glayout::ContainerKind::Row;
}
gview::View ViewBuilder::finish() { return std::move(view_); }

glayout::GraphNode& ViewBuilder::layout(std::string_view id) {
    return *glayout::find_graph_node(view_.layout, id);
}

gview::NodeSpec& ViewBuilder::spec(std::string_view id) {
    const auto found =
        std::find_if(view_.nodes.begin(), view_.nodes.end(),
                     [&](const gview::NodeSpec& item) { return item.layout_id == id; });
    return *found;
}

gview::NodeSpec ViewBuilder::base_spec(std::string id) const {
    gview::NodeSpec spec;
    spec.layout_id = std::move(id);
    const gview::BoxStyle normal = box({7, 23, 27, 238}, {37, 65, 68, 255}, {220, 232, 231, 255});
    spec.style.normal = normal;
    spec.style.selected = box({12, 42, 31, 248}, {89, 133, 96, 255}, {235, 245, 241, 255});
    spec.style.hovered = box({15, 42, 37, 248}, {89, 133, 96, 255}, {235, 245, 241, 255});
    spec.style.focused = box({23, 57, 41, 255}, {151, 239, 117, 255}, {245, 255, 242, 255});
    spec.style.selected_focused =
        box({35, 75, 47, 255}, {190, 255, 161, 255}, {255, 255, 255, 255});
    spec.style.pressed = box({49, 93, 61, 255}, {183, 255, 151, 255}, {255, 255, 255, 255});
    spec.style.disabled = box({9, 19, 22, 200}, {32, 45, 48, 255}, {90, 105, 106, 255});
    spec.text_style.size = 16.0f * scale_;
    spec.text_style.vertical = gview::TextAlign::Center;
    spec.text_style.wrap = true;
    return spec;
}

void ViewBuilder::append(std::string_view parent, glayout::GraphNode node, gview::NodeSpec spec) {
    glayout::GraphNode* target = glayout::find_graph_node(view_.layout, parent);
    target->children.push_back(std::move(node));
    view_.nodes.push_back(std::move(spec));
}

void ViewBuilder::container(std::string_view parent, std::string id, glayout::ContainerKind kind,
                            glayout::Length width, glayout::Length height, float gap,
                            glayout::Insets padding) {
    glayout::GraphNode node;
    node.id = id;
    node.container = kind;
    if (compact() && width.kind == glayout::LengthKind::Pixels && width.value > 300.0f)
        width = {glayout::LengthKind::Percent, 0.44f};
    node.size.width = width;
    node.size.height = height;
    node.gap = gap * scale_;
    node.padding = {padding.left * scale_, padding.top * scale_, padding.right * scale_,
                    padding.bottom * scale_};
    gview::NodeSpec spec = base_spec(id);
    append(parent, std::move(node), std::move(spec));
}

void ViewBuilder::label(std::string_view parent, std::string id, std::string text, float height,
                        float size, gview::TextAlign align) {
    glayout::GraphNode node;
    node.id = id;
    node.size.width = {glayout::LengthKind::Fill, 1.0f};
    node.size.height = {glayout::LengthKind::Pixels, height * scale_};
    node.padding = {12.0f * scale_, 0.0f, 12.0f * scale_, 0.0f};
    gview::NodeSpec spec = base_spec(id);
    spec.content = gview::ContentKind::Text;
    spec.text = std::move(text);
    spec.text_style.size = size * scale_;
    spec.text_style.horizontal = align;
    spec.style.normal.fill = {0, 0, 0, 0};
    spec.style.normal.border = {0, 0, 0, 0};
    append(parent, std::move(node), std::move(spec));
}

void ViewBuilder::button(std::string_view parent, std::string id, std::string text,
                         std::string action, std::string group, float height,
                         gview::ActivationPolicy activation) {
    glayout::GraphNode node;
    node.id = id;
    node.size.width = {glayout::LengthKind::Fill, 1.0f};
    node.size.height = {glayout::LengthKind::Pixels, height * scale_};
    node.padding = {14.0f * scale_, 0.0f, 14.0f * scale_, 0.0f};
    gview::NodeSpec spec = base_spec(id);
    spec.content = gview::ContentKind::Text;
    spec.control = gview::ControlKind::Button;
    spec.text = std::move(text);
    spec.action = std::move(action);
    spec.focus_group = std::move(group);
    spec.focusable = true;
    spec.activation = activation;
    append(parent, std::move(node), std::move(spec));
}

void ViewBuilder::toggle(std::string_view parent, std::string id, std::string text,
                         std::string binding, std::string group, float height) {
    button(parent, id, std::move(text), {}, std::move(group), height);
    gview::NodeSpec& spec = view_.nodes.back();
    spec.control = gview::ControlKind::Toggle;
    spec.binding = std::move(binding);
}

void ViewBuilder::slider(std::string_view parent, std::string id, std::string text,
                         std::string binding, std::string group, double minimum, double maximum,
                         double step, float height) {
    button(parent, id, std::move(text), {}, std::move(group), height);
    gview::NodeSpec& spec = view_.nodes.back();
    spec.control = gview::ControlKind::Slider;
    spec.binding = std::move(binding);
    spec.minimum = minimum;
    spec.maximum = maximum;
    spec.step = step;
}

void ViewBuilder::select(std::string_view parent, std::string id, std::string text,
                         std::string binding, std::vector<std::string> options, std::string group,
                         float height) {
    button(parent, id, std::move(text), {}, std::move(group), height);
    gview::NodeSpec& spec = view_.nodes.back();
    spec.control = gview::ControlKind::Select;
    spec.binding = std::move(binding);
    for (const std::string& option : options)
        spec.options.push_back(gview::SelectOption{option, option, option});
}

void ViewBuilder::text_input(std::string_view parent, std::string id, std::string placeholder,
                             std::string binding, std::string group, float height) {
    button(parent, std::move(id), std::move(placeholder), {}, std::move(group), height);
    gview::NodeSpec& spec = view_.nodes.back();
    spec.control = gview::ControlKind::TextInput;
    spec.binding = std::move(binding);
}

void ViewBuilder::image(std::string_view parent, std::string id, std::string asset, float height) {
    label(parent, id, {}, height);
    view_.nodes.back().content = gview::ContentKind::Image;
    view_.nodes.back().asset = std::move(asset);
}

void ViewBuilder::surface(std::string_view parent, std::string id, std::string asset) {
    container(parent, id, glayout::ContainerKind::Absolute, {glayout::LengthKind::Fill, 1.0f},
              {glayout::LengthKind::Fill, 1.0f});
    gview::NodeSpec& item = spec(id);
    item.content = gview::ContentKind::CustomSurface;
    item.asset = std::move(asset);
    item.style.normal.fill = {0, 0, 0, 0};
    item.style.normal.border = {0, 0, 0, 0};
}

void ViewBuilder::focus_group(std::string id, std::string entry, std::string owner) {
    view_.focus_groups.push_back({std::move(id), std::move(entry), std::move(owner), true, true});
}

void ViewBuilder::edge(std::string from, gview::NavAction action, std::string to) {
    view_.focus_edges.push_back({std::move(from), action, std::move(to)});
}

void ViewBuilder::scrolling(std::string_view id) {
    layout(id).clip = true;
    spec(id).control = gview::ControlKind::ScrollArea;
}
