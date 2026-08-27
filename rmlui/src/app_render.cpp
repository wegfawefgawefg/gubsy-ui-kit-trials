#include "app.h"
#include "focus_elements.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>

void GubsyApp::Render() {
  if (!document_)
    return;
  RenderChrome();
  // remember authored focus action
  Rml::String focused_action;
  if (!pending_focus_action_.empty())
    focused_action = pending_focus_action_;
  else if (Rml::Element *focused = focus_tree::action_element(context_->GetFocusElement()))
    focused_action = focused->GetAttribute<Rml::String>("data-action", "");
  // rebuild active screen markup
  if (Rml::Element *content = document_->GetElementById("screen-content")) {
    content->SetInnerRML(BuildCurrentScreen());
    Rml::ElementList details;
    content->QuerySelectorAll(details, ".detail");
    for (Rml::Element *detail : details) {
      detail->SetAttribute("data-focus", "");
      detail->SetAttribute("data-scroll-region", "");
      detail->SetAttribute("tabindex", "0");
    }
  }
  // restore focus after document replacement
  if (!focused_action.empty()) {
    Rml::ElementList candidates;
    document_->QuerySelectorAll(candidates, "[data-action]");
    for (Rml::Element *candidate : candidates) {
      if (candidate->GetAttribute<Rml::String>("data-action", "") ==
          focused_action) {
        candidate->Focus(true);
        break;
      }
    }
  }
  pending_focus_action_.clear();
  // rebuild guarded modal layer
  if (Rml::Element *modal = document_->GetElementById("modal-root")) {
    if (state_.modal.empty()) {
      modal->SetInnerRML("");
      modal->SetClass("visible", false);
    } else {
      const bool uninstall = state_.modal == "uninstall-plan";
      modal->SetInnerRML(
          std::string(
              R"(<div class="modal-card"><small>CONFIRM LOCAL CHANGE</small><h2>)") +
          (uninstall ? "Uninstall package graph?" : "Are you sure?") +
          (uninstall
               ? R"(</h2><p>Underground Rivers is required by Temple Weather and Pocket Expedition. Confirming removes all three, or cancel to preserve the installed graph.</p>)"
               : R"(</h2><p>This destructive demo action is guarded and remains local to the native prototype.</p>)") +
          R"(<div class="actions"><button data-focus data-action="cancel-modal" class="button">Cancel</button><button data-focus data-action="confirm-modal" class="button danger">Confirm</button></div></div>)");
      modal->SetClass("visible", true);
      if (Rml::Element *cancel =
              modal->QuerySelector("[data-action='cancel-modal']"))
        cancel->Focus(true);
    }
  }
  dirty_ = false;
}

void GubsyApp::RenderChrome() {
  // update fixed shell chrome
  if (!document_)
    return;
  if (Rml::Element *viewport = document_->GetElementById("viewport-size"))
    viewport->SetInnerRML(std::to_string(viewport_width_) + " × " +
                          std::to_string(viewport_height_));
  if (Rml::Element *title = document_->GetElementById("screen-title"))
    title->SetInnerRML(current_screen_name());

  struct NavEntry {
    const char *id;
    Destination destination;
  };
  constexpr NavEntry entries[]{
      {"play", Destination::Play},         {"players", Destination::Players},
      {"settings", Destination::Settings}, {"controls", Destination::Controls},
      {"progress", Destination::Progress}, {"mods", Destination::Mods}};
  for (const NavEntry &entry : entries) {
    if (Rml::Element *element =
            document_->GetElementById(std::string("nav-") + entry.id))
      element->SetClass("selected", state_.destination == entry.destination);
  }
}
