#include "app.h"
#include "focus_elements.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Elements/ElementFormControlInput.h>
#include <RmlUi/Core/Elements/ElementFormControlSelect.h>

#include <algorithm>
#include <cmath>
#include <limits>

void GubsyApp::NavigateFocus(int dx, int dy) {
  if (!document_)
    return;

  // collect visible focus targets
  Rml::ElementList candidates;
  document_->QuerySelectorAll(candidates, state_.modal.empty()
                                              ? "[data-focus]"
                                              : "#modal-root [data-focus]");
  candidates.erase(
      std::remove_if(candidates.begin(), candidates.end(),
                     [](Rml::Element *element) {
                       return !focus_tree::is_visible_focus(element);
                     }),
      candidates.end());
  if (candidates.empty())
    return;

  // define local focus move
  Rml::Element *current = context_->GetFocusElement();
  auto focus = [](Rml::Element *element, bool activate = false) {
    if (!element)
      return;
    element->Focus(true);
    element->ScrollIntoView(false);
    if (activate)
      element->Click();
  };

  // find nearest target in one direction
  auto geometric_target = [&](Rml::Element *origin,
                              const Rml::ElementList &pool) {
    if (!origin)
      return static_cast<Rml::Element *>(nullptr);
    const Rml::Vector2f current_position =
        origin->GetAbsoluteOffset(Rml::BoxArea::Border);
    const float current_left = current_position.x;
    const float current_top = current_position.y;
    const float current_right = current_left + origin->GetOffsetWidth();
    const float current_bottom = current_top + origin->GetOffsetHeight();
    const float cx = (current_left + current_right) * 0.5f;
    const float cy = (current_top + current_bottom) * 0.5f;
    Rml::Element *best = nullptr;
    float best_score = std::numeric_limits<float>::max();
    for (Rml::Element *candidate : pool) {
      if (candidate == origin || !focus_tree::is_visible_focus(candidate) ||
          focus_tree::is_descendant_of(origin, candidate) ||
          focus_tree::is_descendant_of(candidate, origin))
        continue;
      const Rml::Vector2f position =
          candidate->GetAbsoluteOffset(Rml::BoxArea::Border);
      const float left = position.x;
      const float top = position.y;
      const float right = left + candidate->GetOffsetWidth();
      const float bottom = top + candidate->GetOffsetHeight();
      const float x = (left + right) * 0.5f;
      const float y = (top + bottom) * 0.5f;
      const float delta_x = x - cx;
      const float delta_y = y - cy;
      const float forward = delta_x * dx + delta_y * dy;
      if (forward <= 1.0f)
        continue;
      const float cross_overlap =
          dx != 0 ? std::min(current_bottom, bottom) -
                        std::max(current_top, top)
                  : std::min(current_right, right) -
                        std::max(current_left, left);
      if (cross_overlap <= 1.0f)
        continue;
      const float forward_gap =
          dx > 0   ? std::max(0.0f, left - current_right)
          : dx < 0 ? std::max(0.0f, current_left - right)
          : dy > 0 ? std::max(0.0f, top - current_bottom)
                   : std::max(0.0f, current_top - bottom);
      const float lateral = std::abs(delta_x * dy - delta_y * dx);
      const float score = forward_gap * 4.0f + lateral;
      if (score < best_score) {
        best_score = score;
        best = candidate;
      }
    }
    return best;
  };

  // contain modal navigation
  if (!state_.modal.empty()) {
    if (!current || std::find(candidates.begin(), candidates.end(), current) ==
                        candidates.end())
      focus(candidates.front());
    else
      focus(geometric_target(current, candidates));
    return;
  }

  // resolve active primary tab
  const char *active_nav_id = "nav-play";
  switch (state_.destination) {
  case Destination::Players:
    active_nav_id = "nav-players";
    break;
  case Destination::Settings:
    active_nav_id = "nav-settings";
    break;
  case Destination::Controls:
    active_nav_id = "nav-controls";
    break;
  case Destination::Progress:
    active_nav_id = "nav-progress";
    break;
  case Destination::Mods:
    active_nav_id = "nav-mods";
    break;
  case Destination::Play:
    break;
  }
  Rml::Element *active_nav = document_->GetElementById(active_nav_id);
  Rml::Element *main = document_->QuerySelector("main");
  Rml::Element *nav = document_->QuerySelector("nav");
  const bool compact_horizontal_nav =
      viewport_width_ <= 700 ||
      (viewport_height_ <= 500 && viewport_width_ <= 1000);

  const auto is_primary_nav = [](Rml::Element *element) {
    return element &&
           element->GetAttribute<Rml::String>("id", "").rfind("nav-", 0) == 0;
  };
  const auto is_quit = [](Rml::Element *element) {
    return element && element->IsClassSet("quit");
  };
  constexpr const char *nav_ids[]{"nav-play", "nav-players", "nav-settings",
                                  "nav-controls", "nav-progress", "nav-mods"};
  auto primary_index = [&](Rml::Element *element) {
    for (int index = 0; index < 6; ++index) {
      if (element == document_->GetElementById(nav_ids[index]))
        return index;
    }
    return -1;
  };

  if (!current || std::find(candidates.begin(), candidates.end(), current) ==
                      candidates.end()) {
    focus(active_nav);
    return;
  }

  // move primary tabs and activate on focus
  if (is_primary_nav(current)) {
    const int movement = compact_horizontal_nav ? dx : dy;
    const int index = primary_index(current);
    if (movement != 0) {
      const int next = index + (movement > 0 ? 1 : -1);
      if (next >= 0 && next < 6)
        focus(document_->GetElementById(nav_ids[next]), true);
      else if (!compact_horizontal_nav && next == 6) {
        Rml::Element *quit = nav ? nav->QuerySelector("button.quit") : nullptr;
        focus(quit);
      }
      return;
    }
    const bool enter_content = compact_horizontal_nav ? dy < 0 : dx > 0;
    if (enter_content && main)
      FocusRememberedContent();
    return;
  }

  // keep quit outside normal content flow
  if (is_quit(current)) {
    if (!compact_horizontal_nav && dy < 0)
      focus(document_->GetElementById("nav-mods"), true);
    else if (!compact_horizontal_nav && dx > 0 && main) {
      Rml::ElementList content;
      main->QuerySelectorAll(content, "[data-focus]");
      focus(geometric_target(current, content));
    }
    return;
  }

  // move local tabs and activate on focus
  Rml::Element *tab_bar = focus_tree::ancestor_with_class(current, "local-tabs");
  if (!tab_bar)
    tab_bar = focus_tree::ancestor_with_class(current, "mobile-local-tabs");
  if (tab_bar) {
    if (dx != 0) {
      Rml::ElementList tabs;
      tab_bar->QuerySelectorAll(tabs, "[data-focus]");
      const auto it = std::find(tabs.begin(), tabs.end(), current);
      if (it != tabs.end()) {
        const int index = static_cast<int>(it - tabs.begin());
        const int next = index + (dx > 0 ? 1 : -1);
        if (next >= 0 && next < static_cast<int>(tabs.size()))
          focus(tabs[next], true);
      }
      return;
    }
    if (dy > 0 && main) {
      Rml::ElementList content;
      main->QuerySelectorAll(content, "[data-focus]");
      content.erase(std::remove_if(content.begin(), content.end(),
                                   [&](Rml::Element *element) {
                                     return focus_tree::ancestor_with_class(
                                                element, "local-tabs") ||
                                            focus_tree::ancestor_with_class(
                                                element, "mobile-local-tabs");
                                   }),
                    content.end());
      focus(geometric_target(current, content));
    }
    return;
  }

  // contain navigation inside active content
  if (main && focus_tree::is_descendant_of(current, main)) {
    Rml::ElementList content;
    main->QuerySelectorAll(content, "[data-focus]");
    content.erase(
        std::remove_if(content.begin(), content.end(),
                       [](Rml::Element *element) {
                         return !focus_tree::is_visible_focus(element);
                       }),
        content.end());
    Rml::Element *master_detail = focus_tree::ancestor_with_class(current, "master-detail");
    Rml::Element *master_region = focus_tree::ancestor_with_class(current, "master-list");
    Rml::Element *detail_region = focus_tree::ancestor_with_class(current, "detail");
    if (current->IsClassSet("detail"))
      detail_region = current;
    if (dx > 0 && master_detail && master_region) {
      focus(master_detail->QuerySelector(".detail"));
      return;
    }
    // scroll detail regions before leaving them
    if (current->HasAttribute("data-scroll-region") && dy != 0) {
      const float before = current->GetScrollTop();
      current->SetScrollTop(before + static_cast<float>(dy) * 72.0f);
      if (std::abs(current->GetScrollTop() - before) > 0.5f)
        return;
    }
    const std::string current_action =
        current->GetAttribute<Rml::String>("data-action", "");
    if (focus_tree::ancestor_with_class(current, "profile-cards") && dy < 0) {
      focus(main->QuerySelector("[data-action='new-profile']"));
      return;
    }
    if (current_action == "new-profile" && dy != 0) {
      if (dy < 0)
        focus(main->QuerySelector(".local-tabs button.selected"));
      else {
        Rml::Element *selected_profile =
            main->QuerySelector(".profile-card.selected");
        focus(selected_profile ? selected_profile
                               : main->QuerySelector(".profile-card"));
      }
      return;
    }
    // prefer document order inside vertical groups
    if (dy != 0) {
      const bool horizontal_action_row =
          focus_tree::ancestor_with_class(current, "actions") ||
          focus_tree::ancestor_with_class(current, "pair") ||
          focus_tree::ancestor_with_class(current, "detail-actions");
      Rml::Element *focus_group = nullptr;
      if (!horizontal_action_row) {
        focus_group = focus_tree::ancestor_with_class(current, "scroll-body");
        if (!focus_group)
          focus_group = focus_tree::ancestor_with_class(current, "setup-pane");
        if (!focus_group)
          focus_group = focus_tree::ancestor_with_class(current, "party-pane");
        if (!focus_group)
          focus_group = focus_tree::ancestor_with_class(current, "detail");
        if (!focus_group)
          focus_group = focus_tree::ancestor_with_class(current, "master-list");
      }
      if (focus_group) {
        Rml::ElementList ordered;
        focus_group->QuerySelectorAll(ordered, "[data-focus]");
        ordered.erase(
            std::remove_if(ordered.begin(), ordered.end(),
                           [](Rml::Element *element) {
                             return !focus_tree::is_visible_focus(element);
                           }),
            ordered.end());
        const auto it = std::find(ordered.begin(), ordered.end(), current);
        if (it != ordered.end()) {
          const int index = static_cast<int>(it - ordered.begin());
          const int next = index + (dy > 0 ? 1 : -1);
          if (next >= 0 && next < static_cast<int>(ordered.size())) {
            focus(ordered[next]);
            return;
          }
        }
      }
    }
    if (Rml::Element *best = geometric_target(current, content)) {
      focus(best);
      return;
    }
    if (dx < 0 && master_detail && detail_region) {
      Rml::Element *selected =
          master_detail->QuerySelector(".master-list .selected");
      if (!selected)
        selected = master_detail->QuerySelector(".master-list [data-focus]");
      focus(selected);
    }
    return;
  }

  focus(active_nav);
}

void GubsyApp::ActivateFocus() {
  // enter explicit native control editing
  Rml::Element *focus = context_->GetFocusElement();
  if (!focus)
    return;
  if (focus->GetAttribute<Rml::String>("id", "").rfind("nav-", 0) == 0) {
    const bool compact_horizontal_nav =
        viewport_width_ <= 700 ||
        (viewport_height_ <= 500 && viewport_width_ <= 1000);
    if (compact_horizontal_nav)
      NavigateFocus(0, -1);
    else
      NavigateFocus(1, 0);
    return;
  }
  if (focus->HasAttribute("data-scroll-region")) {
    Rml::ElementList controls;
    focus->QuerySelectorAll(controls, "[data-focus]");
    for (Rml::Element *control : controls) {
      if (focus_tree::is_visible_focus(control)) {
        control->Focus(true);
        control->ScrollIntoView(false);
        break;
      }
    }
    return;
  }
  if (auto *select = dynamic_cast<Rml::ElementFormControlSelect *>(focus)) {
    if (select->IsSelectBoxVisible()) {
      select->HideSelectBox();
      controller_editing_ = false;
      select->SetClass("controller-editing", false);
    } else {
      controller_editing_ = true;
      controller_edit_original_value_ = select->GetValue();
      select->SetClass("controller-editing", true);
      select->ShowSelectBox();
    }
    return;
  }
  if (auto *input = dynamic_cast<Rml::ElementFormControlInput *>(focus);
      input) {
    const Rml::String type = input->GetAttribute<Rml::String>("type", "");
    if (type == "checkbox") {
      if (input->HasAttribute("checked"))
        input->RemoveAttribute("checked");
      else
        input->SetAttribute("checked", "checked");
      return;
    }
    if (type == "range") {
      controller_editing_ = true;
      controller_edit_original_value_ = input->GetValue();
      input->SetClass("controller-editing", true);
      SetToast("Adjust with left / right · A accept · B cancel");
      return;
    }
  }
  focus->Click();
}

void GubsyApp::FocusActiveNavigation() {
  // return to active primary tab
  int memory_index = 0;
  const char *id = "nav-play";
  switch (state_.destination) {
  case Destination::Players:
    id = "nav-players";
    memory_index = 1;
    break;
  case Destination::Settings:
    id = "nav-settings";
    memory_index = 2;
    break;
  case Destination::Controls:
    id = "nav-controls";
    memory_index = 3;
    break;
  case Destination::Progress:
    id = "nav-progress";
    memory_index = 4;
    break;
  case Destination::Mods:
    id = "nav-mods";
    memory_index = 5;
    break;
  case Destination::Play:
    break;
  }
  if (Rml::Element *main = document_->QuerySelector("main");
      main && focus_tree::is_descendant_of(context_->GetFocusElement(), main)) {
    if (Rml::Element *action = focus_tree::action_element(context_->GetFocusElement()))
      content_focus_memory_[memory_index] =
          action->GetAttribute<Rml::String>("data-action", "");
  }
  if (Rml::Element *element = document_->GetElementById(id)) {
    element->Focus(true);
    element->ScrollIntoView(false);
  }
}

void GubsyApp::FocusRememberedContent() {
  // restore last content focus
  int memory_index = 0;
  switch (state_.destination) {
  case Destination::Players:
    memory_index = 1;
    break;
  case Destination::Settings:
    memory_index = 2;
    break;
  case Destination::Controls:
    memory_index = 3;
    break;
  case Destination::Progress:
    memory_index = 4;
    break;
  case Destination::Mods:
    memory_index = 5;
    break;
  case Destination::Play:
    break;
  }
  Rml::Element *main = document_->QuerySelector("main");
  if (!main)
    return;
  Rml::ElementList candidates;
  main->QuerySelectorAll(candidates, "[data-focus]");
  Rml::Element *fallback = nullptr;
  for (Rml::Element *candidate : candidates) {
    if (!focus_tree::is_visible_focus(candidate))
      continue;
    if (!fallback)
      fallback = candidate;
    if (!content_focus_memory_[memory_index].empty() &&
        candidate->GetAttribute<Rml::String>("data-action", "") ==
            content_focus_memory_[memory_index]) {
      candidate->Focus(true);
      candidate->ScrollIntoView(false);
      return;
    }
  }
  if (fallback) {
    fallback->Focus(true);
    fallback->ScrollIntoView(false);
  }
}
