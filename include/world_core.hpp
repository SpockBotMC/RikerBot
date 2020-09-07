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

  std::vector<rkr::block_id> get(const std::vector<mcd::mc_position>&
      positions);
  std::vector<rkr::block_id> get(const std::vector<std::array<
      std::int32_t, 3>>& positions);

  private:
  void chunk_update(ev_id_type ev_id, const void* data);
  void chunk_unload(ev_id_type ev_id, const void* data);
  void multiblock_change(ev_id_type ev_id, const void* data);
};



} // namespace rkr

#endif // RKR_WORLD_CORE_HPP
