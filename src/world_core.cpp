#include "world_core.hpp"

namespace rkr {

WorldCore::WorldCore(PluginLoader& ploader, bool ownership) :
    PluginBase("rkr::WorldCore *") {

  ploader.provide("World", this, ownership);
  auto ev = static_cast<EventCore *>(ploader.require("Event"));

  ev->register_callback("ClientboundMapChunk",
      [&](ev_id_type id, const void* p) {chunk_update(id, p);});

  ev->register_callback("ClientboundUnloadChunk",
      [&](ev_id_type id, const void* p) {chunk_unload(id, p);});

  ev->register_callback("ClientboundMultiBlockChange",
      [&](ev_id_type id, const void* p) {multiblock_change(id, p);});
}

std::vector<block_id> WorldCore::get(const std::vector<mcd::mc_position>&
    positions) {
  return world.get(positions);
}
std::vector<block_id> WorldCore::get(const std::vector<std::array<
    std::int32_t, 3>>& positions) {
  return world.get(positions);
}

void WorldCore::chunk_update(ev_id_type ev_id, const void* data) {
  auto packet = static_cast<const mcd::ClientboundMapChunk*>(data);
  world.update(*packet);
}

void WorldCore::chunk_unload(ev_id_type ev_id, const void* data) {
  auto packet = static_cast<const mcd::ClientboundUnloadChunk*>(data);
  world.unload(*packet);
}

void WorldCore::multiblock_change(ev_id_type ev_id,
    const void* data) {
  auto packet = static_cast<const mcd::ClientboundMultiBlockChange*>(data);
  world.update(*packet);
}

} //namespace rkr
