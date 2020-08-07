#ifndef PLUGIN_BASE_HPP
#define PLUGIN_BASE_HPP

#include <string>
#include <optional>

namespace rkr {

class PluginBase {
public:
  const std::optional<std::string> type_query;
  PluginBase() = default;
  PluginBase(std::string type_query);
};

} // namespace rkr
#endif
