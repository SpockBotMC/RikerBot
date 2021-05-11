#ifndef RKR_WORLD_CORE_HPP
#define RKR_WORLD_CORE_HPP

#include "event_core.hpp"
#include "plugin_base.hpp"
#include "smpmap.hpp"

namespace rkr {

class WorldCore : public PluginBase {
public:
  WorldCore(rkr::PluginLoader& ploader, bool ownership = false);

  rkr::block_id get(const rkr::BlockCoord& coord) const;
  rkr::block_id get(std::int32_t x, std::int32_t y, std::int32_t z) const;
  std::vector<rkr::block_id> get(
      const std::vector<mcd::mc_position>& positions) const;
  std::vector<rkr::block_id> get(
      const std::vector<std::array<std::int32_t, 3>>& positions) const;

private:
  rkr::SMPMap world;
  void chunk_update(const void* data);
  void chunk_unload(const void* data);
  void multiblock_change(const void* data);
  void block_change(const void* data);
};

} // namespace rkr

#endif // RKR_WORLD_CORE_HPP
