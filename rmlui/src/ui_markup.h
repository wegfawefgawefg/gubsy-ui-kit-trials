#pragma once

#include <initializer_list>
#include <sstream>
#include <string>
#include <unordered_map>

namespace markup {

std::string selected_class(bool selected, const char *base = "");
std::string selected_class(bool selected, const std::string &base);
std::string on_off(bool value);
bool contains_ci(const std::string &text, const std::string &query);
std::string escape_attribute(const std::string &value);
std::string format_setting_value(const std::string &id,
                                 const std::string &value);
void append_select(std::ostringstream &out, const std::string &action,
                   const std::string &value,
                   std::initializer_list<const char *> options,
                   const char *class_name = "native-select");
std::string map_value(
    const std::unordered_map<std::string, std::string> &values,
    const std::string &id);
std::string mod_image(size_t index);

} // namespace markup
