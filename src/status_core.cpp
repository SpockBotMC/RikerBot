#include "status_core.hpp"
#include "minecraft_protocol.hpp"

namespace rkr {

StatusCore::StatusCore(PluginLoader& ploader, bool ownership)
    : PluginBase("rkr::StatusCore *") {

  ploader.provide("Status", this, ownership);
  ev = static_cast<EventCore*>(ploader.require("Event"));
  io = static_cast<IOCore*>(ploader.require("IO"));

  status_position_update = ev->register_event("status_position_update");
  status_spawn = ev->register_event("status_spawn");

  spawn_cb = ev->register_callback("status_position_update",
      [&](ev_id_type ev_id, const void*) { handle_spawn(ev_id); });

  ev->register_callback("ClientboundPosition",
      [&](ev_id_type, const void* data) { handle_ppl(data); });

  ev->register_callback(
      "tick", [&](ev_id_type, const void*) { handle_tick(); });
}

void StatusCore::handle_spawn(ev_id_type ev_id) {
  ev->emit(status_spawn);
  ev->unregister_callback(ev_id, spawn_cb);
}

void StatusCore::handle_ppl(const void* data) {
  auto packet {static_cast<const mcd::ClientboundPosition*>(data)};
  std::int8_t flags {packet->flags};

  (flags & 0x01) ? position.x += packet->x : position.x = packet->x;
  (flags & 0x02) ? position.y += packet->y : position.y = packet->y;
  (flags & 0x04) ? position.z += packet->z : position.z = packet->z;
  (flags & 0x08) ? look.yaw += packet->yaw : look.yaw = packet->yaw;
  (flags & 0x10) ? look.pitch += packet->pitch : look.pitch = packet->pitch;

  mcd::ServerboundTeleportConfirm resp;
  resp.teleportId = packet->teleportId;
  io->encode_packet(resp);

  position_update_type update {.position = position, .look = look};
  ev->emit(status_position_update, &update, "rkr::position_update_type *");
}

void StatusCore::handle_tick() {
  mcd::ServerboundPositionLook pak;
  pak.x = position.x;
  pak.y = position.y;
  pak.z = position.z;
  pak.yaw = look.yaw;
  pak.pitch = look.pitch;
  pak.onGround = on_ground;
  io->encode_packet(pak);
}

} // namespace rkr
