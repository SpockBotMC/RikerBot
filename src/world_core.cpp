#include "world_core.hpp"

namespace rkr {

WorldCore::WorldCore(PluginLoader& ploader, bool ownership) :
    PluginBase("rkr::WorldCore *") {

  ploader.provide("World", this, ownership);
  auto ev = static_cast<EventCore *>(ploader.require("Event"));

  ev->register_callback("ClientboundMapChunk",
      [&](EventCore::ev_id_type ev_id, const void* data) {
      chunk_update(ev_id, data);});

  ev->register_callback("ClientboundUnloadChunk",
      [&](EventCore::ev_id_type ev_id, const void* data) {
      chunk_unload(ev_id, data);});

  ev->register_callback("ClientboundMultiBlockChange",
      [&](EventCore::ev_id_type ev_id, const void* data) {
      multiblock_change(ev_id, data);});
}

std::vector<block_id> WorldCore::get(const std::vector<mcd::mc_position>&
    positions) {
  return world.get(positions);
}
std::vector<block_id> WorldCore::get(const std::vector<std::array<
    std::int32_t, 3>>& positions) {
  return world.get(positions);
}

void WorldCore::chunk_update(EventCore::ev_id_type ev_id, const void* data) {
  auto packet = static_cast<const mcd::ClientboundMapChunk*>(data);
  world.update(*packet);
}

void WorldCore::chunk_unload(EventCore::ev_id_type ev_id, const void* data) {
  auto packet = static_cast<const mcd::ClientboundUnloadChunk*>(data);
  world.unload(*packet);
}

void WorldCore::multiblock_change(EventCore::ev_id_type ev_id,
    const void* data) {
  auto packet = static_cast<const mcd::ClientboundMultiBlockChange*>(data);
  world.update(*packet);
}

} //namespace rkr
