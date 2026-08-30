#include "app.hpp"
#include "authoring_document.hpp"

#include <algorithm>
#include <cstddef>
#include <filesystem>

namespace {

std::string authoring_root() {
    return std::string(GVIEW_TRIAL_SOURCE_DIR) + "/authoring";
}

std::string shared_theme_path() {
    return authoring_root() + "/shared-theme.sexp";
}
std::string preview_path() {
    return authoring_root() + "/display-simulator.sexp";
}

std::optional<gview::View> load_first_view(const std::filesystem::path& path) {
    const gview::ParseResult parsed = gview::load_view_file(path);
    if (!parsed.ok || parsed.views.empty()) return std::nullopt;
    return parsed.views.front();
}

// What is: The newest legacy page document used once to recover shared theme
// edits made before themes gained their own project-level document.
std::optional<gview::View> newest_legacy_theme() {
    std::filesystem::file_time_type newest{};
    std::optional<gview::View> result;
    std::error_code error;
    for (const auto& entry : std::filesystem::directory_iterator(authoring_root(), error)) {
        if (error || !entry.is_regular_file() || entry.path().extension() != ".sexp" ||
            entry.path().filename() == "shared-theme.sexp" ||
            entry.path().filename() == "display-simulator.sexp")
            continue;
        const auto modified = entry.last_write_time(error);
        if (error || (result && modified <= newest)) continue;
        const auto candidate = load_first_view(entry.path());
        if (!candidate || candidate->themes.empty()) continue;
        newest = modified;
        result = std::move(candidate);
    }
    return result;
}

gview::View theme_document(const std::vector<gview::Theme>& themes, std::string_view active_theme) {
    gview::View result;
    result.id = "shared-theme";
    result.layout.id = "shared-theme-layout";
    result.layout.root.id = "root";
    result.themes = themes;
    result.active_theme = active_theme;
    return result;
}

std::vector<gview::Theme> shared_only(const std::vector<gview::Theme>& themes) {
    std::vector<gview::Theme> result = themes;
    for (gview::Theme& theme : result) {
        std::erase_if(theme.widgets,
                      [](const gview::WidgetSkin& skin) { return !skin.node_id.empty(); });
    }
    return result;
}

bool same_selector(const gview::WidgetSkin& left, const gview::WidgetSkin& right) {
    return left.control == right.control && left.any_control == right.any_control &&
           left.style_class == right.style_class && left.node_id == right.node_id;
}

// What is: New generated recipe slots added without replacing authored slots.
void merge_theme_defaults(std::vector<gview::Theme>& authored,
                          const std::vector<gview::Theme>& generated) {
    for (const gview::Theme& generated_theme : generated) {
        const auto theme = std::find_if(authored.begin(), authored.end(), [&](const auto& item) {
            return item.id == generated_theme.id;
        });
        if (theme == authored.end()) {
            authored.push_back(generated_theme);
            continue;
        }
        for (const gview::WidgetSkin& generated_skin : generated_theme.widgets) {
            const auto skin = std::find_if(theme->widgets.begin(), theme->widgets.end(),
                                           [&](const auto& item) {
                                               return same_selector(item, generated_skin);
                                           });
            if (skin == theme->widgets.end()) {
                theme->widgets.push_back(generated_skin);
                continue;
            }
            for (const gview::PartPresentation& generated_part : generated_skin.parts) {
                const bool present = std::any_of(
                    skin->parts.begin(), skin->parts.end(), [&](const auto& part) {
                        return part.part == generated_part.part && part.state == generated_part.state;
                    });
                if (!present) skin->parts.push_back(generated_part);
            }
        }
    }
}

// What is: One-time repair for the old trial default that confused a toggle's
// stored value with focus by painting the complete row green while it was on.
void remove_legacy_toggle_value_frames(std::vector<gview::Theme>& themes) {
    for (gview::Theme& theme : themes) {
        for (gview::WidgetSkin& skin : theme.widgets) {
            if (skin.any_control || skin.control != gview::ControlKind::Toggle ||
                !skin.style_class.empty() || !skin.node_id.empty())
                continue;
            std::erase_if(skin.parts, [](const gview::PartPresentation& part) {
                const bool old_on = part.state == gview::PresentationState::On &&
                                    part.asset == "ui-action-green";
                const bool old_off = part.state == gview::PresentationState::Off &&
                                     part.asset == "ui-button-light";
                return part.part == gview::WidgetPart::Frame && (old_on || old_off);
            });
        }
    }
}

void append_local_overrides(std::vector<gview::Theme>& destination,
                            const std::vector<gview::Theme>& source) {
    for (const gview::Theme& local_theme : source) {
        const auto found =
            std::find_if(destination.begin(), destination.end(),
                         [&](const auto& item) { return item.id == local_theme.id; });
        if (found == destination.end()) continue;
        for (const gview::WidgetSkin& skin : local_theme.widgets)
            if (!skin.node_id.empty()) found->widgets.push_back(skin);
    }
}

void link_legacy_slice_geometry(std::vector<gview::Theme>& themes) {
    for (gview::Theme& theme : themes) {
        for (gview::WidgetSkin& skin : theme.widgets) {
            for (std::size_t index = 0; index < skin.parts.size(); ++index) {
                const gview::PartPresentation& source = skin.parts[index];
                if (source.image_mode != gview::ImageMode::NineSlice) continue;
                const bool handled = std::any_of(
                    skin.parts.begin(), skin.parts.begin() + static_cast<std::ptrdiff_t>(index),
                    [&](const auto& prior) {
                        return prior.part == source.part && prior.asset == source.asset;
                    });
                if (handled) continue;
                for (gview::PartPresentation& target : skin.parts) {
                    if (target.part != source.part || target.asset != source.asset ||
                        target.image_mode != gview::ImageMode::NineSlice)
                        continue;
                    target.slice = source.slice;
                    target.slice_margins = source.slice_margins;
                    target.slice_scale = source.slice_scale;
                    target.slice_modes = source.slice_modes;
                }
            }
        }
    }
}

} // namespace

// What is: One canonical theme library shared by every authored page.
void TrialApp::initialize_shared_theme(const gview::View& generated) {
    if (shared_theme_initialized_) return;
    const std::vector<gview::Theme> defaults = shared_only(generated.themes);
    shared_themes_ = defaults;
    shared_active_theme_ = generated.active_theme;
    std::optional<gview::View> source = load_first_view(shared_theme_path());
    const bool migrating_legacy = !source;
    if (!source) source = newest_legacy_theme();
    if (source && !source->themes.empty()) {
        shared_themes_ = shared_only(source->themes);
        merge_theme_defaults(shared_themes_, defaults);
        remove_legacy_toggle_value_frames(shared_themes_);
        shared_active_theme_ = std::move(source->active_theme);
        if (migrating_legacy) link_legacy_slice_geometry(shared_themes_);
    }
    shared_theme_initialized_ = true;
}

void TrialApp::capture_shared_theme() {
    if (authoring_.view().themes.empty()) return;
    shared_themes_ = shared_only(authoring_.view().themes);
    remove_legacy_toggle_value_frames(shared_themes_);
    shared_active_theme_ = authoring_.view().active_theme;
    shared_theme_initialized_ = true;
}

void TrialApp::apply_shared_theme(gview::View& view) const {
    if (!shared_theme_initialized_ || shared_themes_.empty()) return;
    const std::vector<gview::Theme> local = view.themes;
    view.themes = shared_themes_;
    append_local_overrides(view.themes, local);
    view.active_theme = shared_active_theme_;
}

bool TrialApp::save_authoring_documents() {
    capture_shared_theme();
    apply_shared_theme(authoring_.view());
    const bool page_saved =
        authoring_.save() || authoring_.save_as(authoring_path(authoring_context_));
    const bool theme_saved = gview::save_view_file(
        shared_theme_path(), {theme_document(shared_themes_, shared_active_theme_)});
    for (auto& [id, document] : authoring_documents_) {
        (void)id;
        apply_shared_theme(document.view());
    }
    return page_saved && theme_saved;
}

bool TrialApp::reload_authoring_documents() {
    if (!authoring_.reload()) return false;
    migrate_authored_view(authoring_.view(), generated_view_);
    const auto shared = load_first_view(shared_theme_path());
    if (shared && !shared->themes.empty()) {
        shared_themes_ = shared->themes;
        remove_legacy_toggle_value_frames(shared_themes_);
        shared_active_theme_ = shared->active_theme;
        shared_theme_initialized_ = true;
    } else {
        capture_shared_theme();
    }
    apply_shared_theme(authoring_.view());
    for (auto& [id, document] : authoring_documents_) {
        (void)id;
        apply_shared_theme(document.view());
    }
    return true;
}

void TrialApp::restore_authoring_preferences() {
    if (authoring_preferences_restored_) return;
    if (const auto preview = gview::load_preview_config(preview_path())) {
        authoring_ui_.preview = *preview;
        resize(preview->width, preview->height);
        painter_->set_device_pixel_ratio(preview->device_pixel_ratio);
        painter_->set_nearest_sampling(preview->sampling == gview::PreviewSampling::Nearest);
    }
    authoring_preferences_restored_ = true;
}

void TrialApp::apply_preview(const gview::PreviewConfig& preview) {
    resize(preview.width, preview.height);
    painter_->set_device_pixel_ratio(preview.device_pixel_ratio);
    painter_->set_nearest_sampling(preview.sampling == gview::PreviewSampling::Nearest);
    if (authoring_preferences_restored_) gview::save_preview_config(preview_path(), preview);
}
