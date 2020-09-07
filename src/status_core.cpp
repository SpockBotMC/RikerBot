#include "status_core.hpp"

namespace rkr {

StatusCore::StatusCore(PluginLoader& ploader, bool ownership) :
    PluginBase("rkr::StatusCore *") {
  ploader.provide("Status", this, ownership);
}

} // namespace rkr
