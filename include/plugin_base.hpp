#ifndef PLUGIN_BASE_HPP
#define PLUGIN_BASE_HPP

#include <optional>
#include <string>

namespace rkr {

class PluginBase {
public:
  const std::optional<std::string> type_query;
  PluginBase(std::string type_query = "");
};

} // namespace rkr
#endif
