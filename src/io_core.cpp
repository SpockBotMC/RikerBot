#include <iostream>
#include <algorithm>
#include <functional>

#include "datautils.hpp"
#include "io_core.hpp"

namespace rkr {

IOCore::IOCore(PluginLoader& ploader) : state(mcd::HANDSHAKING),
    compression(COMPRESSION_DISABLED), kill(0), sock(ctx), rslv(ctx),
    in_is(&in_buf) {
  ploader.provide_class("io", this);
  ev = static_cast<EventCore*>(ploader.get_class("event"));
  in_is.exceptions(in_is.eofbit | in_is.badbit | in_is.failbit);
  connect_event = ev->register_event("net_connect");
  kill_event = ev->register_event("kill");
  ev->register_callback("ClientboundEncryptionBegin",
      [&](EventCore::ev_id_type ev_id, const void* data) {
      encryption_begin_handler(ev_id, data);});
  for(int state_itr = 0; state_itr < mcd::STATE_MAX; state_itr++) {
    for(int dir_itr = 0; dir_itr < mcd::DIRECTION_MAX; dir_itr++) {
      for(int id = 0; id < mcd::protocol_max_ids[state_itr][dir_itr]; id++) {
        auto name = mcd::protocol_cstrings[state_itr][dir_itr][id];
        auto event_id = ev->register_event(name);
        packet_event_ids[state_itr][dir_itr].push_back(event_id);
      }
    }
  }
}

void IOCore::run() {
  while(!kill) {
    ctx.run();
    if(out_buf.size())
      net::async_write(sock, out_buf.data(), [&](const sys::error_code& ec,
          std::size_t len) {write_handler(ec, len);});
    // 5 bytes is the maximum size of the packet length header, but it's
    // possible for an entire packet to be shorter than that. So we prepare
    // a 5 byte buffer then use read_some to read however many bytes come.
    sock.async_read_some(in_buf.prepare(5), [&](const sys::error_code& ec,
        std::size_t len) {header_handler(ec, len);});
  }
  ev->emit(kill_event);
}

void IOCore::encode_packet(const mcd::Packet& packet) {
  static std::ostream pak_os(&pak_buf);
  static std::ostream out_os(&out_buf);

  if(compression == COMPRESSION_DISABLED) {
    mcd::enc_varint(pak_os, packet.packet_id);
    packet.encode(pak_os);
    size_t packet_size = pak_buf.size();
    mcd::enc_varint(out_os, packet_size);
    auto mutable_buf = out_buf.prepare(packet_size);
    std::memcpy(mutable_buf.data(), pak_buf.data().data(), packet_size);
    out_buf.commit(packet_size);
    pak_buf.consume(packet_size);
  } else {
    throw std::runtime_error("Compression Unimplemented");
  }

  ev->emit(packet_event_ids[state][mcd::SERVERBOUND][packet.packet_id],
      static_cast<const void*>(&packet), "mcd::" + packet.name + " *");
}

void IOCore::connect(std::string host, std::string service) {
  auto endpoints = rslv.resolve(host, service);
  net::async_connect(sock, endpoints, [&](const sys::error_code& ec,
      const ip::tcp::endpoint& ep) {connect_handler(ec, ep);});
}

void IOCore::connect_handler(const sys::error_code& ec,
    const ip::tcp::endpoint& ep) {
  if(ec.failed()) {
    std::cout << ec.message() << std::endl;
    exit(-1);
  }

  ConnectData data {ep.address().to_string(), ep.port()};
  ev->emit(connect_event, static_cast<const void*>(&data),
      "rkr::ConnectData *");
}

void IOCore::write_handler(const sys::error_code& ec, std::size_t len) {
  if(ec.failed()) {
    std::cout << ec.message() << std::endl;
    exit(-1);
  }
  out_buf.consume(len);
}

void IOCore::header_handler(const sys::error_code& ec, std::size_t len) {
  if(ec.failed()) {
    std::cout << ec.message() << std::endl;
    exit(-1);
  }
  in_buf.commit(len);
  int varnum = mcd::verify_varint(
      static_cast<const char *>(in_buf.data().data()), in_buf.size());
  if(varnum == mcd::VARNUM_INVALID) {
    exit(-1);
  } else if (varnum == mcd::VARNUM_OVERRUN) {
    sock.async_read_some(in_buf.prepare(5 - in_buf.size()),
        [&](const sys::error_code& ec, std::size_t len)
        {header_handler(ec, len);});
  } else {
    auto varint = mcd::dec_varint(in_is);
    auto mutable_buf = in_buf.prepare(varint - in_buf.size());
    net::async_read(sock, mutable_buf, [&](const sys::error_code& ec,
        std::size_t len) {read_packet_handler(ec, len);});
  }
}

void IOCore::read_packet_handler(const sys::error_code& ec, std::size_t len) {
  if(ec.failed()) {
    std::cout << ec.message() << std::endl;
    exit(-1);
  }
  in_buf.commit(len);
  if(compression == COMPRESSION_DISABLED) {
    std::int64_t packet_id;
    try {packet_id = mcd::dec_varint(in_is);}
    catch(std::exception&) {
      std::cout << "Failed to decode packet id" << std::endl;
      exit(-1);
    }
    auto packet = mcd::make_packet(state, mcd::CLIENTBOUND, packet_id);
    try {packet->decode(in_is);}
    catch(std::exception&) {
      std::cout << "Failed to decode packet, suspect id: " << packet_id;
      std::cout << std::endl;
      exit(-1);
    }

    ev->emit(packet_event_ids[state][mcd::CLIENTBOUND][packet->packet_id],
        static_cast<const void*>(packet.get()), "mcd::" + packet->name + " *");
  } else {
    throw std::runtime_error("Compression Unimplemented");
  }
}

void IOCore::encryption_begin_handler(EventCore::ev_id_type ev_id,
    const void* data) {

}

} // namespace rkr
