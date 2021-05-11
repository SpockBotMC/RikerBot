#ifndef RKR_STATUS_CORE_HPP
#define RKR_STATUS_CORE_HPP

#include "event_core.hpp"
#include "io_core.hpp"
#include "plugin_base.hpp"
#include "plugin_loader.hpp"

namespace rkr {

struct position_type {
  double x;
  double y;
  double z;
};

struct look_type {
  float yaw;
  float pitch;
};

struct position_update_type {
  position_type position;
  look_type look;
};

class StatusCore : public PluginBase {
public:
  position_type position;
  look_type look;
  bool on_ground;

  StatusCore(rkr::PluginLoader& ploader, bool ownership = false);

private:
  EventCore* ev;
  IOCore* io;
  ev_id_type status_position_update;
  ev_id_type status_spawn;
  cb_id_type spawn_cb;
  void handle_spawn(ev_id_type ev_id);
  void handle_ppl(const void* data);
  void handle_tick();
};

} // namespace rkr

#endif // RKR_STATUS_CORE_HPP
