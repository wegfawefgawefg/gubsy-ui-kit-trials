#include "ui_markup.h"

#include <algorithm>
#include <cctype>

namespace markup {

std::string selected_class(bool selected, const char *base) {
  std::string result(base);
  if (selected)
    result += result.empty() ? "selected" : " selected";
  return result;
}

std::string selected_class(bool selected, const std::string &base) {
  return selected_class(selected, base.c_str());
}

std::string on_off(bool value) { return value ? "ON" : "OFF"; }

bool contains_ci(const std::string &text, const std::string &query) {
  if (query.empty())
    return true;
  const auto fold = [](unsigned char character) {
    return std::tolower(character);
  };
  return std::search(text.begin(), text.end(), query.begin(), query.end(),
                     [&](char left, char right) {
                       return fold(static_cast<unsigned char>(left)) ==
                              fold(static_cast<unsigned char>(right));
                     }) != text.end();
}

std::string escape_attribute(const std::string &value) {
  std::string result;
  result.reserve(value.size());
  for (const char character : value) {
    switch (character) {
    case '&':
      result += "&amp;";
      break;
    case '"':
      result += "&quot;";
      break;
    case '<':
      result += "&lt;";
      break;
    case '>':
      result += "&gt;";
      break;
    default:
      result += character;
      break;
    }
  }
  return result;
}

std::string format_setting_value(const std::string &id,
                                 const std::string &value) {
  if (value == "true")
    return "ON";
  if (value == "false" || value.empty())
    return "OFF";
  if (id == "brightness" || id.find("volume") != std::string::npos ||
      id == "camera-shake" || id == "render-scale")
    return value + "%";
  return value;
}

void append_select(std::ostringstream &out, const std::string &action,
                   const std::string &value,
                   std::initializer_list<const char *> options,
                   const char *class_name) {
  out << R"(<select data-focus data-action=")" << action << R"(" class=")"
      << class_name << R"(">)";
  for (const char *option : options) {
    out << R"(<option value=")" << escape_attribute(option) << R"(")";
    if (value == option)
      out << R"( selected="selected")";
    out << R"(>)" << option << R"(</option>)";
  }
  out << R"(</select>)";
}

std::string map_value(
    const std::unordered_map<std::string, std::string> &values,
    const std::string &id) {
  const auto found = values.find(id);
  return found == values.end() ? std::string{} : found->second;
}

std::string mod_image(size_t index) {
  return "sheet-" + std::to_string(index / 5 + 1) + "-" +
         std::to_string(index % 5) + ".png";
}

} // namespace markup
