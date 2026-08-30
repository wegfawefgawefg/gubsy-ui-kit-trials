#include "app.hpp"
#include "authoring_document.hpp"
#include "theme.hpp"
#include "view_builder.hpp"

#include <algorithm>
#include <cstdio>

namespace {

bool expect(bool condition, const char* message) {
    if (!condition) std::fprintf(stderr, "self-test failed: %s\n", message);
    return condition;
}

bool expect_focus(const TrialApp& app, std::string_view expected, const char* message) {
    const std::string actual = app.focus_id();
    if (actual == expected) return true;
    std::fprintf(stderr, "self-test failed: %s (expected %.*s, got %s)\n", message,
                 static_cast<int>(expected.size()), expected.data(), actual.c_str());
    return false;
}

void step(TrialApp& app, gview::NavAction action) {
    app.navigate(action);
    app.update();
    app.update();
}

void key_step(TrialApp& app, SDL_Keycode key) {
    SDL_Event event{};
    event.type = SDL_EVENT_KEY_DOWN;
    event.key.key = key;
    app.process(event);
    app.update();
    app.update();
}

// What is: Authored geometry survives while the host refreshes semantic content.
bool expect_authored_merge() {
    TrialModel model;
    model.destination = Destination::Mods;
    model.mods_tab = "Installed";
    gview::View authored = build_shell_view(model, 1280, 720);
    glayout::GraphNode* detail = glayout::find_graph_node(authored.layout, "mod-detail");
    if (!detail) return expect(false, "mod detail exists for merge coverage");
    detail->size.width = {glayout::LengthKind::Pixels, 503.0f};
    std::swap(detail->children[0], detail->children[1]);

    model.selected_mod = "Base Content";
    const gview::View generated = build_shell_view(model, 1280, 720);
    const gview::View merged = merge_authored_view(authored, generated);
    const glayout::GraphNode* merged_detail = glayout::find_graph_node(merged.layout, "mod-detail");
    const auto name =
        std::find_if(merged.nodes.begin(), merged.nodes.end(),
                     [](const gview::NodeSpec& node) { return node.layout_id == "mod-name"; });
    return expect(merged_detail && merged_detail->size.width.value == 503.0f,
                  "runtime refresh retains authored constraints") &&
           expect(merged_detail && merged_detail->children[0].id == "mod-kicker",
                  "runtime refresh retains authored sibling order") &&
           expect(name != merged.nodes.end() && name->text == "Base Content",
                  "runtime refresh updates host-owned content");
}

// What is: Presentation-state coverage for compound controls whose value and
// interaction visuals must remain independent.
bool expect_compound_control_theme() {
    const std::vector<gview::Theme> themes = trial_themes();
    gview::NodeSpec toggle;
    toggle.control = gview::ControlKind::Toggle;
    const gview::CompiledSkin toggle_skin =
        gview::compile_skin(themes, "gubsy-default", toggle);
    const gview::PartPresentation* toggle_on =
        gview::find_part(toggle_skin, gview::WidgetPart::Frame,
                         gview::PresentationState::On);
    const gview::PartPresentation* indicator_on =
        gview::find_part(toggle_skin, gview::WidgetPart::Indicator,
                         gview::PresentationState::On);

    gview::NodeSpec select;
    select.control = gview::ControlKind::Select;
    const gview::CompiledSkin select_skin =
        gview::compile_skin(themes, "gubsy-default", select);
    const gview::PartPresentation* popup =
        gview::find_part(select_skin, gview::WidgetPart::Popup,
                         gview::PresentationState::Normal);
    const gview::PartPresentation* option =
        gview::find_part(select_skin, gview::WidgetPart::Option,
                         gview::PresentationState::Normal);
    const gview::PartPresentation* selected =
        gview::find_part(select_skin, gview::WidgetPart::Frame,
                         gview::PresentationState::Selected);
    const gview::PartPresentation* selected_focused =
        gview::find_part(select_skin, gview::WidgetPart::Frame,
                         gview::PresentationState::SelectedFocused);
    return expect(toggle_on && toggle_on->asset == "ui-button-light",
                  "an on toggle keeps its neutral unfocused row") &&
           expect(indicator_on && indicator_on->asset == "ui-toggle-on",
                  "toggle value remains visible on its indicator") &&
           expect(popup && popup->asset == "ui-action-green",
                  "open select uses an action-green popup enclosure") &&
           expect(option && option->asset == "ui-button-light",
                  "ordinary select options remain light") &&
           expect(selected && selected->asset == "ui-action-green-dark",
                  "persistent selection uses subdued green") &&
           expect(selected_focused && selected_focused->asset == "ui-action-green",
                  "active selected focus uses bright green");
}

// What is: Shell structure and themed scroll gutters remain distinct from
// visible panel recipes.
bool expect_structural_shell_geometry() {
    TrialModel play_model;
    const gview::View play = build_shell_view(play_model, 1280, 720);
    const glayout::GraphNode* main = glayout::find_graph_node(play.layout, "main");
    const bool content_is_structural =
        std::none_of(play.nodes.begin(), play.nodes.end(), [](const auto& node) {
            return node.layout_id == "content";
        });

    gview::View legacy = play;
    glayout::GraphNode* legacy_main = glayout::find_graph_node(legacy.layout, "main");
    legacy_main->padding.left = 32.0f;
    legacy_main->padding.right = 32.0f;
    legacy_main->padding.bottom = 12.0f;
    gview::NodeSpec old_content;
    old_content.layout_id = "content";
    old_content.style_class = "parchment-ornate";
    legacy.nodes.push_back(old_content);
    migrate_authored_view(legacy, play);
    legacy_main = glayout::find_graph_node(legacy.layout, "main");
    const bool legacy_removed =
        std::none_of(legacy.nodes.begin(), legacy.nodes.end(), [](const auto& node) {
            return node.layout_id == "content";
        });

    TrialModel controls_model;
    controls_model.destination = Destination::Controls;
    const gview::View controls = build_shell_view(controls_model, 1280, 720);
    const glayout::GraphNode* list = glayout::find_graph_node(controls.layout, "action-list");
    return expect(main && main->padding.left == 16.0f && main->padding.right == 16.0f &&
                      main->padding.bottom == 22.0f,
                  "desktop workspace uses the balanced quit-button gutter") &&
           expect(legacy_main && legacy_main->padding.left == 16.0f &&
                      legacy_main->padding.right == 16.0f && legacy_main->padding.bottom == 22.0f,
                  "legacy authored pages adopt the balanced workspace gutter") &&
           expect(content_is_structural, "shell content does not paint a redundant frame") &&
           expect(legacy_removed, "legacy authored content presentation is retired") &&
           expect(list && list->clip && list->padding.left >= 18.0f &&
                      list->padding.right >= 23.0f && list->padding.bottom >= 23.0f,
                  "scroll lists reserve a themed content gutter");
}

} // namespace

// Exercises controller semantics and real widgets without moving the user's
// devices.
bool run_self_test(TrialApp& app) {
    bool ok = expect_authored_merge() && expect_compound_control_theme() &&
              expect_structural_shell_geometry();
    for (int screen = 0; screen <= 17; ++screen) {
        app.select_screen(screen);
        app.update();
        ok &= expect(!app.focus_id().empty(), "every authored state has an initial focus target");
        if (screen < 17) {
            const std::string owner = app.focus_id();
            step(app, gview::NavAction::Right);
            ok &= expect(app.focus_id() != owner, "every shell state enters its local focus group");
        }
    }
    app.select_screen(0);
    app.update();
    ok &= expect(app.focus_id() == "nav-Play", "Play owns initial focus");
    key_step(app, SDLK_D);
    ok &= expect_focus(app, "activity", "D produces semantic right navigation");
    key_step(app, SDLK_A);
    ok &= expect_focus(app, "nav-Play", "A produces semantic left navigation");
    key_step(app, SDLK_S);
    ok &= expect_focus(app, "nav-Players", "S produces semantic down navigation");
    key_step(app, SDLK_W);
    ok &= expect_focus(app, "nav-Play", "W produces semantic up navigation");
    app.set_authoring_enabled(true, false);
    key_step(app, SDLK_F1);
    ok &= expect(!app.authoring_enabled(), "F1 hides the complete authoring layer");
    key_step(app, SDLK_F1);
    ok &= expect(app.authoring_enabled(), "F1 restores the complete authoring layer");
    app.set_authoring_enabled(false, false);
    step(app, gview::NavAction::Right);
    ok &= expect(app.focus_id() == "activity", "right enters Play setup");
    step(app, gview::NavAction::Left);
    ok &= expect(app.focus_id() == "nav-Play", "left exits the Play setup group to its rail owner");
    step(app, gview::NavAction::Right);
    step(app, gview::NavAction::Down);
    ok &= expect(app.focus_id() == "resume-point", "down follows local setup order");
    step(app, gview::NavAction::Back);
    ok &= expect(app.focus_id() == "nav-Play", "back returns to active destination");
    step(app, gview::NavAction::Confirm);
    ok &= expect(app.focus_id() == "resume-point", "confirm restores remembered child");
    step(app, gview::NavAction::Back);
    step(app, gview::NavAction::Down);
    ok &= expect(app.focus_id() == "nav-Players", "rail navigation activates destination");
    app.select_screen(4);
    app.update();
    step(app, gview::NavAction::Right);
    step(app, gview::NavAction::Down);
    ok &= expect(app.focus_id() == "roster-moss", "Players tab enters its local content");
    step(app, gview::NavAction::Right);
    ok &= expect(app.focus_id() == "player-profile", "roster enters the adjacent detail group");
    step(app, gview::NavAction::Left);
    ok &= expect(app.focus_id() == "roster-moss", "detail returns to the remembered roster item");
    step(app, gview::NavAction::Down);
    step(app, gview::NavAction::Right);
    step(app, gview::NavAction::Down);
    step(app, gview::NavAction::Left);
    ok &= expect(app.focus_id() == "roster-vega",
                 "cross-group memory restores the exact roster item last used");
    app.select_screen(4);
    app.update();
    step(app, gview::NavAction::Right);
    step(app, gview::NavAction::Right);
    ok &= expect_focus(app, "player-tab-Profiles", "right stays within the Players tab strip");

    app.select_screen(0);
    app.update();
    step(app, gview::NavAction::Right);
    step(app, gview::NavAction::Up);
    for (int index = 0; index < 5; ++index)
        step(app, gview::NavAction::Down);
    ok &= expect_focus(app, "session-mods", "Play setup reaches its final list item");
    step(app, gview::NavAction::Down);
    ok &= expect(app.focus_id() == "pause-preview", "setup enters the separate action row");
    step(app, gview::NavAction::Right);
    ok &= expect(app.focus_id() == "begin-session", "both Play actions are reachable locally");
    step(app, gview::NavAction::Up);
    ok &=
        expect(app.focus_id() == "session-mods", "action row returns to the remembered setup item");

    app.select_screen(0);
    app.update();
    step(app, gview::NavAction::Right);
    step(app, gview::NavAction::Right);
    step(app, gview::NavAction::Confirm);
    ok &= expect_focus(app, "nav-Players", "party player opens the Players destination");

    app.select_screen(3);
    app.update();
    step(app, gview::NavAction::Right);
    step(app, gview::NavAction::Down);
    step(app, gview::NavAction::Down);
    ok &= expect_focus(app, "session-Cartographer's Desk",
                       "session package list owns local movement");
    step(app, gview::NavAction::Confirm);
    ok &= expect_focus(app, "browse-add", "confirm enters the session package action pane");
    step(app, gview::NavAction::Back);
    ok &=
        expect_focus(app, "session-Cartographer's Desk", "back restores the exact session package");

    app.select_screen(7);
    app.update();
    step(app, gview::NavAction::Right);
    step(app, gview::NavAction::Down);
    step(app, gview::NavAction::Down);
    ok &= expect(app.focus_id() == "setting-resolution", "settings controls are reachable");
    const gview::Value resolution = app.value("resolution");
    step(app, gview::NavAction::Confirm);
    ok &= expect(app.focus_open(), "select opens instead of cycling");
    step(app, gview::NavAction::Down);
    ok &= expect(app.value("resolution") == resolution, "open select does not commit on movement");
    step(app, gview::NavAction::Confirm);
    ok &= expect(app.value("resolution") != resolution, "select commits explicitly");

    app.select_screen(13);
    app.update();
    step(app, gview::NavAction::Right);
    step(app, gview::NavAction::Down);
    const double sensitivity = std::get<double>(app.value("look-sensitivity"));
    step(app, gview::NavAction::Right);
    ok &= expect(std::get<double>(app.value("look-sensitivity")) > sensitivity,
                 "controller adjusts a real slider");

    app.select_screen(16);
    app.update();
    step(app, gview::NavAction::Right);
    step(app, gview::NavAction::Down);
    step(app, gview::NavAction::Confirm);
    app.enter_text("cavern");
    app.update();
    step(app, gview::NavAction::Confirm);
    ok &= expect(std::get<std::string>(app.value("catalog-search")) == "cavern",
                 "text input commits typed state");
    step(app, gview::NavAction::Down);
    step(app, gview::NavAction::Down);
    ok &= expect_focus(app, "catalog-item-1", "catalog selection is local to the package list");
    step(app, gview::NavAction::Confirm);
    ok &= expect_focus(app, "install-session", "catalog confirm enters the detail actions");
    step(app, gview::NavAction::Back);
    ok &= expect_focus(app, "catalog-item-1", "catalog detail returns to the remembered package");

    app.select_screen(16);
    app.update();
    step(app, gview::NavAction::Right);
    step(app, gview::NavAction::Down);
    step(app, gview::NavAction::Down);
    for (int index = 0; index < 12; ++index)
        step(app, gview::NavAction::Down);
    const std::string catalog_focus = app.focus_id();
    const float catalog_scroll = app.scroll_offset("catalog-list");
    ok &= expect(app.node_selected(catalog_focus),
                 "focus-previewed catalog package becomes the persistent selection");
    step(app, gview::NavAction::Right);
    ok &= expect(catalog_scroll > 0.0f, "deep catalog focus scrolls the package list") &&
          expect(app.scroll_offset("catalog-list") == catalog_scroll,
                 "leaving the catalog list preserves its scroll position") &&
          expect(app.node_selected(catalog_focus),
                 "catalog selection survives entry into the detail pane");
    step(app, gview::NavAction::Up);
    ok &= expect(app.node_selected(catalog_focus),
                 "refocusing the active Mods tab does not reset its package selection");
    step(app, gview::NavAction::Down);
    step(app, gview::NavAction::Down);
    ok &= expect_focus(app, catalog_focus, "deep catalog focus remains the remembered package");

    app.select_screen(15);
    app.update();
    step(app, gview::NavAction::Right);
    step(app, gview::NavAction::Down);
    step(app, gview::NavAction::Down);
    for (int index = 0; index < 6; ++index)
        step(app, gview::NavAction::Down);
    ok &= expect_focus(app, "installed-6", "controller reaches the final installed package");
    ok &= expect(app.scroll_offset("installed-list") > 0.0f,
                 "host-content rebuild retains focus-driven list scroll");

    app.select_screen(17);
    app.update();
    ok &= expect(app.focus_id() == "inventory-item-8", "non-menu inventory has stable grid focus");
    step(app, gview::NavAction::Right);
    ok &= expect(app.focus_id() == "inventory-item-9",
                 "inventory keeps local grid movement before a scope exit");
    step(app, gview::NavAction::Right);
    step(app, gview::NavAction::Right);
    step(app, gview::NavAction::Right);
    ok &= expect(app.focus_id() == "item-use", "grid edge enters its local action group");
    step(app, gview::NavAction::Back);
    ok &= expect(app.focus_id() == "inventory-item-11",
                 "inventory back restores the exact item left behind");
    if (ok) std::puts("gview trial self-test passed");
    return ok;
}
