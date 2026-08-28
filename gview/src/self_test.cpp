#include "app.hpp"

#include <cstdio>

namespace {

bool expect(bool condition, const char* message) {
    if (!condition) std::fprintf(stderr, "self-test failed: %s\n", message);
    return condition;
}

void step(TrialApp& app, gview::NavAction action) {
    app.navigate(action);
    app.update();
    app.update();
}

} // namespace

// Exercises controller semantics and real widgets without moving the user's
// devices.
bool run_self_test(TrialApp& app) {
    bool ok = true;
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
    step(app, gview::NavAction::Right);
    ok &= expect(app.focus_id() == "activity", "right enters Play setup");
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
