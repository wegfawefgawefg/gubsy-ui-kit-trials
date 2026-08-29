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

// What is: One generated 32 px panel cut rendered at control-scale borders.
gview::PartPresentation nine_slice(std::string asset, gview::PresentationState state,
                                   float border_scale, float opacity = 1.0f) {
    gview::PartPresentation result =
        asset_part(gview::WidgetPart::Frame, std::move(asset), gview::ImageMode::NineSlice, opacity);
    result.state = state;
    result.slice = 32.0f;
    result.slice_scale = border_scale;
    result.draw_box_underlay = false;
    return result;
}

gview::WidgetSkin slider_skin() {
    gview::WidgetSkin skin;
    skin.control = gview::ControlKind::Slider;
    skin.parts = {
        asset_part(gview::WidgetPart::Track, "ui-slider-track", gview::ImageMode::NineSlice, 0.82f,
                   {42, 88, 96, 255}),
        asset_part(gview::WidgetPart::Fill, "ui-slider-fill", gview::ImageMode::NineSlice, 1.0f,
                   {142, 239, 117, 255}),
        asset_part(gview::WidgetPart::Thumb, "ui-slider-thumb", gview::ImageMode::Contain)};
    skin.parts[0].slice = 16.0f;
    skin.parts[1].slice = 16.0f;
    skin.parts[0].slice_modes.top = gview::SliceTileMode::Repeat;
    skin.parts[0].slice_modes.bottom = gview::SliceTileMode::Repeat;
    skin.parts[1].slice_modes.top = gview::SliceTileMode::Repeat;
    skin.parts[1].slice_modes.bottom = gview::SliceTileMode::Repeat;
    return skin;
}

gview::WidgetSkin control_skin(gview::ControlKind control) {
    gview::WidgetSkin skin;
    skin.control = control;
    constexpr float button_border = 0.22f;
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
        control_skin(gview::ControlKind::TextInput), slider_skin()};
    gview::Theme game;
    game.id = "splonks";
    game.extends = "gubsy-default";
    game.widgets = {
        region_skin("bar-dark", "ui-bar-dark", 0.25f),
        region_skin("parchment-ornate", "ui-parchment-ornate", 0.28f),
        region_skin("group-inner", "ui-group-inner", 0.24f),
    };
    return {std::move(base), std::move(game)};
}
