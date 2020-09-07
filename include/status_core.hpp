#ifndef RKR_STATUS_CORE_HPP
#define RKR_STATUS_CORE_HPP

#include "plugin_loader.hpp"
#include "plugin_base.hpp"

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

class StatusCore : public PluginBase {
public:
  position_type position;
  look_type look;
  bool on_ground;

  StatusCore(rkr::PluginLoader& ploader, bool ownership = false);

private:
};


} // namespace rkr


#endif // RKR_STATUS_CORE_HPP
