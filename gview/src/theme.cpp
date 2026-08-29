#include "theme.hpp"

namespace {

gview::PartPresentation asset_part(gview::WidgetPart part, std::string asset,
                                   gview::ImageMode mode, float opacity = 1.0f,
                                   gview::Color tint = {255, 255, 255, 255}) {
    gview::PartPresentation result;
    result.part = part;
    result.asset = std::move(asset);
    result.image_mode = mode;
    result.opacity = opacity;
    result.tint = tint;
    return result;
}

gview::WidgetSkin slider_skin() {
    gview::WidgetSkin skin;
    skin.control = gview::ControlKind::Slider;
    skin.parts = {
        asset_part(gview::WidgetPart::Track, "ui-slider-track",
                   gview::ImageMode::NineSlice, 0.82f, {42, 88, 96, 255}),
        asset_part(gview::WidgetPart::Fill, "ui-slider-fill",
                   gview::ImageMode::NineSlice, 1.0f, {142, 239, 117, 255}),
        asset_part(gview::WidgetPart::Thumb, "ui-slider-thumb", gview::ImageMode::Contain)};
    skin.parts[0].slice = 16.0f;
    skin.parts[1].slice = 16.0f;
    return skin;
}

gview::WidgetSkin control_skin(gview::ControlKind control) {
    gview::WidgetSkin skin;
    skin.control = control;
    skin.parts = {asset_part(gview::WidgetPart::Frame, "ui-control-frame",
                             gview::ImageMode::NineSlice, 0.28f)};
    skin.parts[0].slice = 12.0f;
    return skin;
}

gview::WidgetSkin panel_skin() {
    gview::WidgetSkin skin;
    skin.any_control = true;
    skin.style_class = "panel";
    skin.parts = {asset_part(gview::WidgetPart::Frame, "ui-panel-grid",
                             gview::ImageMode::Cover, 0.42f)};
    return skin;
}

} // namespace

// Defines the neutral trial skin through the same public recipe API as games.
std::vector<gview::Theme> trial_themes() {
    gview::Theme base;
    base.id = "gubsy-default";
    base.widgets = {control_skin(gview::ControlKind::Button),
                    control_skin(gview::ControlKind::Toggle),
                    control_skin(gview::ControlKind::Slider),
                    control_skin(gview::ControlKind::Select),
                    control_skin(gview::ControlKind::TextInput), slider_skin()};
    gview::Theme game;
    game.id = "splonks";
    game.extends = "gubsy-default";
    game.widgets = {panel_skin()};
    return {std::move(base), std::move(game)};
}
