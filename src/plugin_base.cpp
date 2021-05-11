#include "plugin_base.hpp"

namespace rkr {

PluginBase::PluginBase(std::string type_query)
    : type_query(std::in_place, type_query) {}

} // namespace rkr
