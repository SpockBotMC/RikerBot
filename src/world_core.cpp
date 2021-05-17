#include "world_core.hpp"

namespace rkr {

WorldCore::WorldCore(PluginLoader& ploader, bool ownership)
    : PluginBase("rkr::WorldCore *") {

  ploader.provide("World", this, ownership);
  auto ev {static_cast<EventCore*>(ploader.require("Event"))};

  ev->register_callback("ClientboundMapChunk",
      [&](ev_id_type, const void* data) { chunk_update(data); });

  ev->register_callback("ClientboundUnloadChunk",
      [&](ev_id_type, const void* data) { chunk_unload(data); });

  ev->register_callback("ClientboundMultiBlockChange",
      [&](ev_id_type, const void* data) { multiblock_change(data); });

  ev->register_callback("ClientboundBlockChange",
      [&](ev_id_type, const void* data) { block_change(data); });
}

block_id WorldCore::get(const BlockCoord& coord) const {
  return world.get(coord);
}
block_id WorldCore::get(std::int32_t x, std::int32_t y, std::int32_t z) const {
  return world.get(x, y, z);
}
std::vector<block_id> WorldCore::get(
    const std::vector<mcd::mc_position>& coords) const {
  return world.get(coords);
}
std::vector<block_id> WorldCore::get(
    const std::vector<std::array<std::int32_t, 3>>& coords) const {
  return world.get(coords);
}

void WorldCore::chunk_update(const void* data) {
  world.update(*static_cast<const mcd::ClientboundMapChunk*>(data));
}

void WorldCore::chunk_unload(const void* data) {
  world.unload(*static_cast<const mcd::ClientboundUnloadChunk*>(data));
}

void WorldCore::multiblock_change(const void* data) {
  world.update(*static_cast<const mcd::ClientboundMultiBlockChange*>(data));
}

void WorldCore::block_change(const void* data) {
  world.update(*static_cast<const mcd::ClientboundBlockChange*>(data));
}

} // namespace rkr
