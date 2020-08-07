#include "start_plugin.hpp"

namespace rkr {

StartPlugin::StartPlugin(PluginLoader& ploader) {
  ev = static_cast<EventCore*>(ploader.get_class("event"));
  io = static_cast<IOCore*>(ploader.get_class("io"));

  ev->register_callback("net_connect", [&](EventCore::ev_id_type ev_id,
      const void* data) {start_login(ev_id, data);});
}

void StartPlugin::start_login(EventCore::ev_id_type ev_id, const void* data) {
  auto addr = static_cast<const std::pair<std::string, std::uint16_t>*>(
      data);
  auto set_p = mcd::ServerboundSetProtocol();
  set_p.protocolVersion = MC_PROTO_VERSION;
  set_p.nextState = mcd::LOGIN;
  set_p.serverHost = addr->first;
  set_p.serverPort = addr->second;
  io->encode_packet(set_p);
  io->encode_packet(mcd::ServerboundLoginStart());
  io->state = mcd::LOGIN;
}

}
