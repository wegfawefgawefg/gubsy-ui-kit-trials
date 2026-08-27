#pragma once

namespace Rml {
class Element;
}

namespace focus_tree {

Rml::Element *action_element(Rml::Element *element);
bool is_descendant_of(Rml::Element *element, Rml::Element *ancestor);
Rml::Element *ancestor_with_class(Rml::Element *element,
                                  const char *class_name);
bool is_visible_focus(Rml::Element *element);

} // namespace focus_tree
