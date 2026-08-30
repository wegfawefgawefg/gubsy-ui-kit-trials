#include "theme.hpp"

namespace {

gview::PartPresentation asset_part(gview::WidgetPart part, std::string asset, gview::ImageMode mode,
                                   float opacity = 1.0f, gview::Color tint = {255, 255, 255, 255}) {
    gview::PartPresentation result;
    result.part = part;
    result.asset = std::move(asset);
    result.image_mode = mode;
    result.opacity = opacity;
    result.tint = tint;
    return result;
}

// What is: Three-slice-style control strips expressed through asymmetric
// nine-slice source caps.
gview::PartPresentation sliced_part(gview::WidgetPart part, std::string asset, float left,
                                    float top, float right, float bottom) {
    gview::PartPresentation result =
        asset_part(part, std::move(asset), gview::ImageMode::NineSlice);
    result.slice_margins = {left, top, right, bottom};
    result.draw_box_underlay = false;
    return result;
}

// What is: One generated 23 px cut rendered at its authored source scale.
gview::PartPresentation nine_slice_part(gview::WidgetPart part, std::string asset,
                                        gview::PresentationState state, float border_scale,
                                        float opacity = 1.0f) {
    gview::PartPresentation result =
        asset_part(part, std::move(asset), gview::ImageMode::NineSlice, opacity);
    result.state = state;
    result.slice = 23.0f;
    result.slice_scale = border_scale;
    result.draw_box_underlay = false;
    return result;
}

// What is: Normal control and region frame shorthand.
gview::PartPresentation nine_slice(std::string asset, gview::PresentationState state,
                                   float border_scale, float opacity = 1.0f) {
    return nine_slice_part(gview::WidgetPart::Frame, std::move(asset), state, border_scale,
                           opacity);
}

gview::WidgetSkin slider_skin() {
    gview::WidgetSkin skin;
    skin.control = gview::ControlKind::Slider;
    skin.parts = {sliced_part(gview::WidgetPart::Track, "ui-slider-track", 8.0f, 3.0f,
                              8.0f, 3.0f),
                  sliced_part(gview::WidgetPart::Fill, "ui-slider-fill", 8.0f, 3.0f, 8.0f,
                              3.0f),
                  asset_part(gview::WidgetPart::Thumb, "ui-slider-thumb",
                             gview::ImageMode::Contain)};
    skin.parts[2].draw_box_underlay = false;
    return skin;
}

// What is: Value imagery independent from the toggle row's hover and focus.
gview::WidgetSkin toggle_indicator_skin() {
    gview::WidgetSkin skin;
    skin.control = gview::ControlKind::Toggle;
    auto off = asset_part(gview::WidgetPart::Indicator, "ui-toggle-off",
                          gview::ImageMode::Contain);
    off.state = gview::PresentationState::Off;
    off.draw_box_underlay = false;
    auto on = asset_part(gview::WidgetPart::Indicator, "ui-toggle-on", gview::ImageMode::Contain);
    on.state = gview::PresentationState::On;
    on.draw_box_underlay = false;
    skin.parts = {std::move(off), std::move(on)};
    return skin;
}

// What is: Passive scrollbar presentation shared by every clipped scroll area.
gview::WidgetSkin scrollbar_skin() {
    gview::WidgetSkin skin;
    skin.control = gview::ControlKind::ScrollArea;
    skin.parts = {
        sliced_part(gview::WidgetPart::Track, "ui-scrollbar-track", 4.0f, 8.0f, 4.0f, 8.0f),
        sliced_part(gview::WidgetPart::Thumb, "ui-scrollbar-thumb", 4.0f, 8.0f, 4.0f, 8.0f)};
    return skin;
}

gview::WidgetSkin control_skin(gview::ControlKind control) {
    gview::WidgetSkin skin;
    skin.control = control;
    constexpr float button_border = 1.0f;
    skin.parts = {
        nine_slice("ui-button-light", gview::PresentationState::Normal, button_border),
        nine_slice("ui-button-light", gview::PresentationState::Hovered, button_border),
        nine_slice("ui-action-green", gview::PresentationState::Focused, button_border),
        nine_slice("ui-action-green", gview::PresentationState::Selected, button_border),
        nine_slice("ui-action-green", gview::PresentationState::SelectedFocused, button_border),
        nine_slice("ui-action-green", gview::PresentationState::Pressed, button_border),
        nine_slice("ui-action-green", gview::PresentationState::Open, button_border),
        nine_slice("ui-action-green", gview::PresentationState::On, button_border),
        nine_slice("ui-button-light", gview::PresentationState::Off, button_border),
        nine_slice("ui-button-light", gview::PresentationState::Disabled, button_border, 0.55f)};
    if (control == gview::ControlKind::Select) {
        auto popup = nine_slice_part(gview::WidgetPart::Popup, "ui-group-inner",
                                     gview::PresentationState::Normal, button_border);
        popup.outset = 6.0f;
        skin.parts.push_back(std::move(popup));
        skin.parts.push_back(nine_slice_part(gview::WidgetPart::Option, "ui-button-light",
                                             gview::PresentationState::Normal, button_border));
        skin.parts.push_back(nine_slice_part(gview::WidgetPart::Option, "ui-action-green",
                                             gview::PresentationState::Selected, button_border));
    }
    return skin;
}

// What is: A semantic region recipe shared by every matching layout node.
gview::WidgetSkin region_skin(std::string style_class, std::string asset, float border_scale) {
    gview::WidgetSkin skin;
    skin.any_control = true;
    skin.style_class = std::move(style_class);
    skin.parts = {nine_slice(std::move(asset), gview::PresentationState::Normal, border_scale)};
    return skin;
}

} // namespace

// Defines the neutral trial skin through the same public recipe API as games.
std::vector<gview::Theme> trial_themes() {
    gview::Theme base;
    base.id = "gubsy-default";
    base.widgets = {
        control_skin(gview::ControlKind::Button),    control_skin(gview::ControlKind::Toggle),
        control_skin(gview::ControlKind::Slider),    control_skin(gview::ControlKind::Select),
        control_skin(gview::ControlKind::TextInput), slider_skin(), toggle_indicator_skin(),
        scrollbar_skin()};
    gview::Theme game;
    game.id = "splonks";
    game.extends = "gubsy-default";
    game.widgets = {
        region_skin("bar-dark", "ui-bar-dark", 1.0f),
        region_skin("parchment-ornate", "ui-parchment-ornate", 1.0f),
        region_skin("group-inner", "ui-group-inner", 1.0f),
    };
    return {std::move(base), std::move(game)};
}
