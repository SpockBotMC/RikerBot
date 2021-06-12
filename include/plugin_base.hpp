#ifndef PLUGIN_BASE_HPP
#define PLUGIN_BASE_HPP

#include <optional>
#include <string>

namespace rkr {

struct PluginBase {
  const std::optional<std::string> type_query;
  PluginBase() = default;
  PluginBase(std::string type_query)
      : type_query {std::in_place, type_query} {}
};

} // namespace rkr
#endif
