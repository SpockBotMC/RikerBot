#ifndef RKR_WORLD_CORE_HPP
#define RKR_WORLD_CORE_HPP

#include "plugin_base.hpp"
#include "event_core.hpp"
#include "smpmap.hpp"

namespace rkr {

class WorldCore : public PluginBase {
public:
  rkr::SMPMap world;

  WorldCore(rkr::PluginLoader& ploader, bool ownership = false);

  private:
  void chunk_update(EventCore::ev_id_type ev_id, const void* data);
  void chunk_unload(EventCore::ev_id_type ev_id, const void* data);
  void multiblock_change(EventCore::ev_id_type ev_id, const void* data);
};



} // namespace rkr

#endif // RKR_WORLD_CORE_HPP
