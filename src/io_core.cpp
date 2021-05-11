// I am 31, which is very young for my age.
// That is enough to realize I’m a pencil that has learned
// how to draw the Internet. I explain squiggles
// diagramming exactly how I feel and you are drawn to read
// in ways you cannot yet. Slow goes the drag
// of creation, how what’s within comes to be without,
// which is the rhythmic erection of essence.
//   - Amy King, Wings of Desire

#include <algorithm>
#include <boost/log/trivial.hpp>
#include <botan/data_src.h>
#include <botan/pubkey.h>
#include <botan/system_rng.h>
#include <botan/x509_key.h>
#include <chrono>
#include <functional>

#include "datautils.hpp"
#include "io_core.hpp"

namespace rkr {

IOCore::IOCore(PluginLoader& ploader, net::io_context& ctx, bool ownership)
    : PluginBase {"rkr::IOCore *"}, sock {ctx}, rslv {ctx}, tick_timer {ctx} {

  read_is.exceptions(read_is.eofbit | read_is.badbit | read_is.failbit);
  Botan::system_rng().randomize(shared_secret, std::size(shared_secret));

  inflateInit(&inflator);
  deflateInit(&deflator, Z_DEFAULT_COMPRESSION);

  ploader.provide("IO", this, ownership);
  ev = static_cast<EventCore*>(ploader.require("Event"));
  connect_event = ev->register_event("io_connect");
  kill_event = ev->register_event("kill");
  tick_event = ev->register_event("tick");

  ev->register_callback("ServerboundSetProtocol",
      [&](ev_id_type, const void* data) { transition_state(data); });

  ev->register_callback("auth_session_success",
      [&](ev_id_type, const void* data) { encryption_begin_handler(data); });

  ev->register_callback("ServerboundEncryptionBegin",
      [&](ev_id_type, const void* data) { enable_encryption(data); });

  ev->register_callback("ClientboundCompress",
      [&](ev_id_type, const void* data) { enable_compression(data); });

  ev->register_callback("ClientboundSuccess",
      [&](ev_id_type, const void* data) { login_success(data); });

  ev->register_callback("status_spawn", //
      [&](ev_id_type, const void*) { tick(); });

  for(int state_itr = 0; state_itr < mcd::STATE_MAX; state_itr++) {
    for(int dir_itr = 0; dir_itr < mcd::DIRECTION_MAX; dir_itr++) {
      for(int id = 0; id < mcd::protocol_max_ids[state_itr][dir_itr]; id++) {
        auto name {mcd::protocol_cstrings[state_itr][dir_itr][id]};
        auto event_id {ev->register_event(name)};
        packet_event_ids[state_itr][dir_itr].push_back(event_id);
      }
    }
  }
}

void IOCore::tick() {
  ev->emit(tick_event);
  tick_timer.expires_after(std::chrono::milliseconds(50));
  tick_timer.async_wait([&](const sys::error_code&) { tick(); });
}

void IOCore::read_packet() {
  if(read_buf.size()) {
    read_header();
  } else {
    // 5 bytes is the maximum size of the packet length header, but it's
    // possible for an entire packet to be shorter than that. So we prepare
    // a 5 byte buffer then use read_some to read however many bytes come.
    in_buf = read_buf.prepare(5);
    sock.async_read_some(
        in_buf, [&](const sys::error_code& ec, std::size_t len) {
          header_handler(ec, len);
        });
  }
}

void IOCore::write_packet(
    const boost::asio::streambuf& header, const boost::asio::streambuf& body) {
  out_bufs.push_back(header.data());
  out_bufs.push_back(body.data());
  if(!ongoing_write) {
    ongoing_write = true;
    net::async_write(
        sock, out_bufs, [&](const sys::error_code& ec, std::size_t len) {
          write_handler(ec, len);
        });
  }
}

void IOCore::encode_packet(const mcd::Packet& packet) {
  auto& header_buf {write_bufs.emplace_back()};
  auto& body_buf {write_bufs.emplace_back()};
  std::ostream header_os {&header_buf}, body_os {&body_buf};

  mcd::enc_varint(body_os, packet.packet_id);
  packet.encode(body_os);
  auto packet_size {body_buf.size()};

  if(compressed && packet_size > threshold) {
    // Worst case scenario for a single-digit length packet being compressed
    // due to an unreasonable compression threshold. Shouldn't be more than
    // 11 bytes of overhead.
    auto avail_out {packet_size + 11};
    // Beware, C-style casts ahead
    deflator.next_out = (Bytef*) body_buf.prepare(avail_out).data();
    deflator.avail_out = avail_out;
    deflator.next_in = (unsigned char*) body_buf.data().data();
    deflator.avail_in = packet_size;
    while(deflate(&deflator, Z_FINISH) != Z_STREAM_END) {
      body_buf.commit(avail_out);
      deflator.next_out = (Bytef*) body_buf.prepare(avail_out).data();
      deflator.avail_out = avail_out;
    }
    deflateReset(&deflator);
    body_buf.commit(avail_out - deflator.avail_out);
    body_buf.consume(packet_size);

    auto compressed_size {body_buf.size()};
    auto total_size {compressed_size + mcd::size_varint(packet_size)};

    mcd::enc_varint(header_os, total_size);
    mcd::enc_varint(header_os, packet_size);
  } else {
    if(compressed) {
      mcd::enc_varint(header_os, packet_size + 1);
      mcd::enc_byte(header_os, 0);
    } else {
      mcd::enc_varint(header_os, packet_size);
    }
  }

  if(encrypted) {
    // Botan will only let me use a CipherMode in-place. So we do a bad thing
    // and discard const. Blame Botan.
    encryptor->process(
        static_cast<std::uint8_t*>(const_cast<void*>(header_buf.data().data())),
        header_buf.size());

    encryptor->process(
        static_cast<std::uint8_t*>(const_cast<void*>(body_buf.data().data())),
        body_buf.size());
  }

  ev->emit(packet_event_ids[state][mcd::SERVERBOUND][packet.packet_id],
      static_cast<const void*>(&packet), "mcd::" + packet.packet_name + " *");

  write_packet(header_buf, body_buf);
}

void IOCore::connect(const std::string& host, const std::string& service) {
  auto endpoints = rslv.resolve(host, service);
  net::async_connect(sock, endpoints,
      [&](const sys::error_code& ec, const ip::tcp::endpoint& ep) {
        connect_handler(ec, ep);
      });
}

void IOCore::connect_handler(
    const sys::error_code& ec, const ip::tcp::endpoint& ep) {
  if(ec.failed()) {
    BOOST_LOG_TRIVIAL(fatal) << ec.message();
    exit(-1);
  }

  compressed = false;
  encrypted = false;
  ConnectData data {ep.address().to_string(), ep.port()};
  ev->emit(
      connect_event, static_cast<const void*>(&data), "rkr::ConnectData *");
  read_packet();
}

void IOCore::write_handler(const sys::error_code& ec, std::size_t len) {
  if(ec.failed()) {
    BOOST_LOG_TRIVIAL(fatal) << ec.message();
    exit(-1);
  }

  while(len) {
    len -= write_bufs.front().size();
    write_bufs.pop_front();
    out_bufs.pop_front();
  }

  if(!out_bufs.empty())
    net::async_write(
        sock, out_bufs, [&](const sys::error_code& ec, std::size_t len) {
          write_handler(ec, len);
        });
  else
    ongoing_write = false;
}

void IOCore::read_header() {
  auto varnum {mcd::verify_varint(
      static_cast<const char*>(read_buf.data().data()), read_buf.size())};

  if(varnum == mcd::VARNUM_INVALID) {
    BOOST_LOG_TRIVIAL(fatal) << "Invalid header";
    exit(-1);
  } else if(varnum == mcd::VARNUM_OVERRUN) {
    in_buf = read_buf.prepare(5 - read_buf.size());
    sock.async_read_some(
        in_buf, [&](const sys::error_code& ec, std::size_t len) {
          header_handler(ec, len);
        });
  } else {
    auto varint {mcd::dec_varint(read_is)};
    if(read_buf.size() >= static_cast<std::uint64_t>(varint)) {
      read_body(varint);
      return;
    }
    in_buf = read_buf.prepare(varint - read_buf.size());
    net::async_read(
        sock, in_buf, [&, varint](const sys::error_code& ec, std::size_t len) {
          body_handler(ec, len, varint);
        });
  }
}

void IOCore::header_handler(const sys::error_code& ec, std::size_t len) {
  if(ec.failed()) {
    BOOST_LOG_TRIVIAL(fatal) << ec.message();
    exit(-1);
  }
  if(encrypted)
    decryptor->process(static_cast<std::uint8_t*>(in_buf.data()), len);
  read_buf.commit(len);
  read_header();
}

void IOCore::body_handler(
    const sys::error_code& ec, std::size_t len, int32_t body_len) {
  if(ec.failed()) {
    BOOST_LOG_TRIVIAL(fatal) << ec.message();
    exit(-1);
  }
  if(encrypted)
    decryptor->process(static_cast<std::uint8_t*>(in_buf.data()), len);
  read_buf.commit(len);
  read_body(body_len);
}

void IOCore::read_body(size_t len) {
  static boost::asio::streambuf pak_buf;
  static std::istream pak_is {&pak_buf};

  auto orig_size {read_buf.size()};
  int64_t uncompressed_len;

  if(compressed) {
    uncompressed_len = mcd::dec_varint(read_is);
    if(uncompressed_len) {
      auto remaining_buf {len - (orig_size - read_buf.size())};

      inflator.next_out =
          (unsigned char*) pak_buf.prepare(uncompressed_len).data();
      inflator.avail_out = uncompressed_len;
      inflator.next_in = (unsigned char*) read_buf.data().data();
      inflator.avail_in = remaining_buf;

      if(auto err {inflate(&inflator, Z_FINISH)}; err != Z_STREAM_END) {
        BOOST_LOG_TRIVIAL(fatal) << "Err: " << err << " " << inflator.msg;
        exit(-1);
      }

      inflateReset(&inflator);
      pak_buf.commit(uncompressed_len);
      read_buf.consume(remaining_buf);
    }
  }

  std::istream& is_ref {compressed && uncompressed_len ? pak_is : read_is};
  auto packet_id {static_cast<int>(mcd::dec_varint(is_ref))};
  std::unique_ptr<mcd::Packet> packet;

  try {
    packet = mcd::make_packet(state, mcd::CLIENTBOUND, packet_id);
  } catch(std::exception&) {
    BOOST_LOG_TRIVIAL(fatal) << "Invalid packet id";
    exit(-1);
  }
  packet->decode(is_ref);
  // Needs to be exception based, otherwise reads can cause infinite loops
  if((compressed && (pak_is.eof() || pak_is.fail() || pak_buf.size())) ||
      (!compressed && (len != orig_size - read_buf.size()))) {
    BOOST_LOG_TRIVIAL(fatal)
        << "Failed to decode packet, Suspect ID: " << packet_id
        << " Suspect name: " << packet->packet_name;
    exit(-1);
  }
  ev->emit(packet_event_ids[state][mcd::CLIENTBOUND][packet->packet_id],
      static_cast<const void*>(packet.get()),
      "mcd::" + packet->packet_name + " *");

  read_packet();
}

void IOCore::encryption_begin_handler(const void* data) {
  auto packet {static_cast<const mcd::ClientboundEncryptionBegin*>(data)};

  Botan::DataSource_Memory mem {
      reinterpret_cast<const std::uint8_t*>(packet->publicKey.data()),
      packet->publicKey.size()};

  auto& rng {Botan::system_rng()};
  std::unique_ptr<Botan::Public_Key> key {Botan::X509::load_key(mem)};
  Botan::PK_Encryptor_EME enc {*key, rng, "PKCS1v15"};

  mcd::ServerboundEncryptionBegin resp;

  // This nonsense is necessary because char and uint8_t vectors don't play
  // nicely with one another. It is absolutely a standards violation.
  auto rslt {enc.encrypt(shared_secret, std::size(shared_secret), rng)};
  resp.sharedSecret = reinterpret_cast<std::vector<char>&&>(rslt);

  rslt = enc.encrypt(
      reinterpret_cast<const std::uint8_t*>(packet->verifyToken.data()),
      packet->verifyToken.size(), rng);
  resp.verifyToken = reinterpret_cast<std::vector<char>&&>(rslt);

  encode_packet(resp);
}

void IOCore::enable_encryption(const void* data) {
  encryptor->clear();
  encryptor->set_key(shared_secret, std::size(shared_secret));
  encryptor->start(shared_secret, std::size(shared_secret));
  decryptor->clear();
  decryptor->set_key(shared_secret, std::size(shared_secret));
  decryptor->start(shared_secret, std::size(shared_secret));
  encrypted = true;
}

void IOCore::enable_compression(const void* data) {
  threshold = static_cast<const mcd::ClientboundCompress*>(data)->threshold;
  compressed = true;
}

void IOCore::transition_state(const void* data) {
  auto packet {static_cast<const mcd::ServerboundSetProtocol*>(data)};
  state = static_cast<mcd::packet_state>(packet->nextState);
}

void IOCore::login_success(const void* data) {
  state = mcd::PLAY;
}

} // namespace rkr
