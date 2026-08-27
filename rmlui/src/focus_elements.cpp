#include "focus_elements.h"

#include <RmlUi/Core/Element.h>

namespace focus_tree {

Rml::Element *action_element(Rml::Element *element) {
  while (element) {
    if (element->HasAttribute("data-action"))
      return element;
    element = element->GetParentNode();
  }
  return nullptr;
}

bool is_descendant_of(Rml::Element *element, Rml::Element *ancestor) {
  for (; element; element = element->GetParentNode()) {
    if (element == ancestor)
      return true;
  }
  return false;
}

Rml::Element *ancestor_with_class(Rml::Element *element,
                                  const char *class_name) {
  for (; element; element = element->GetParentNode()) {
    if (element->IsClassSet(class_name))
      return element;
  }
  return nullptr;
}

bool is_visible_focus(Rml::Element *element) {
  return element && element->GetOffsetWidth() > 0 &&
         element->GetOffsetHeight() > 0;
}

} // namespace focus_tree
