#ifndef START_PLUGIN_HPP
#define START_PLUGIN_HPP

#include "plugin_loader.hpp"
#include "event_core.hpp"
#include "io_core.hpp"

namespace rkr {

class StartPlugin {
public:
  StartPlugin(rkr::PluginLoader& ploader);

private:
  EventCore* ev;
  IOCore* io;
  std::string username;
  void start_login(EventCore::ev_id_type ev_id, const void* data);
};

} // namespace rkr
#endif
