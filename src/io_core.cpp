#include <algorithm>
#include <functional>
#include <botan/system_rng.h>
#include <botan/pubkey.h>
#include <botan/data_src.h>
#include <botan/x509_key.h>
#include <boost/log/trivial.hpp>

#include "datautils.hpp"
#include "io_core.hpp"


namespace rkr {

IOCore::IOCore(PluginLoader& ploader, bool ownership) :
    PluginBase("rkr::IOCore *"), kill(0), state(mcd::HANDSHAKING),
    compressed(false), encrypted(false), sock(ctx), rslv(ctx),
    out_os(&out_buf), in_is(&in_buf) {

  in_is.exceptions(in_is.eofbit | in_is.badbit | in_is.failbit);
  Botan::system_rng().randomize(shared_secret, std::size(shared_secret));

  encryptor = Botan::Cipher_Mode::create("AES-128/CFB/8", Botan::ENCRYPTION);
  decryptor = Botan::Cipher_Mode::create("AES-128/CFB/8", Botan::DECRYPTION);
  inflator = {};
  inflateInit(&inflator);
  deflator = {};
  deflateInit(&deflator, Z_DEFAULT_COMPRESSION);

  ploader.provide("IO", this, ownership);
  ev = static_cast<EventCore*>(ploader.require("Event"));
  connect_event = ev->register_event("io_connect");
  kill_event = ev->register_event("kill");

  ev->register_callback("ServerboundSetProtocol",
      [&](EventCore::ev_id_type ev_id, const void* data) {
      transition_state(ev_id, data);});

  ev->register_callback("ClientboundEncryptionBegin",
      [&](EventCore::ev_id_type ev_id, const void* data) {
      encryption_begin_handler(ev_id, data);});

  ev->register_callback("ServerboundEncryptionBegin",
      [&](EventCore::ev_id_type ev_id, const void* data) {
      enable_encryption(ev_id, data);});

  ev->register_callback("ClientboundCompress",
      [&](EventCore::ev_id_type ev_id, const void* data) {
      enable_compression(ev_id, data);});

  ev->register_callback("ClientboundSuccess",
      [&](EventCore::ev_id_type ev_id, const void* data) {
      login_success(ev_id, data);});

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
    ctx.restart();
    ctx.run();
    if(out_buf.size())
      net::async_write(sock, out_buf.data(), [&](const sys::error_code& ec,
          std::size_t len) {write_handler(ec, len);});
    // Last packet was so small that we already have data in the in_buf
    if(in_buf.size())
      read_header();
    else {
      // 5 bytes is the maximum size of the packet length header, but it's
      // possible for an entire packet to be shorter than that. So we prepare
      // a 5 byte buffer then use read_some to read however many bytes come.
      read_buf = in_buf.prepare(5);
      sock.async_read_some(read_buf, [&](const sys::error_code& ec,
          std::size_t len) {header_handler(ec, len);});
    }
  }
  ev->emit(kill_event);
}

void IOCore::encode_packet(const mcd::Packet& packet) {
  static std::ostream pak_os(&pak_buf);

  static boost::asio::streambuf write_buf;
  static std::ostream write_os(&write_buf);

  mcd::enc_varint(pak_os, packet.packet_id);
  packet.encode(pak_os);
  auto packet_size = pak_buf.size();

  if(compressed && packet_size > threshold) {
    // Worst case scenario for a single-digit length packet being compressed
    // due to an unreasonable compression threshold. Shouldn't be more than
    // 11 bytes of overhead.
    auto avail_out = packet_size + 11;
    // Beware, C-style casts ahead
    deflator.next_out = (Bytef*) pak_buf.prepare(avail_out).data();
    deflator.avail_out = avail_out;
    deflator.next_in = (unsigned char*) pak_buf.data().data();
    deflator.avail_in = packet_size;
    while(deflate(&deflator, Z_FINISH) != Z_STREAM_END) {
      pak_buf.commit(avail_out);
      deflator.next_out = (Bytef*) pak_buf.prepare(avail_out).data();
      deflator.avail_out = avail_out;
    }
    deflateReset(&deflator);
    pak_buf.commit(avail_out - deflator.avail_out);
    pak_buf.consume(packet_size);

    auto compressed_size = pak_buf.size();
    int32_t total_size = compressed_size + mcd::size_varint(packet_size);
    mcd::enc_varint(write_os, total_size);
    mcd::enc_varint(write_os, packet_size);
    std::memcpy(write_buf.prepare(compressed_size).data(),
        pak_buf.data().data(), compressed_size);
    write_buf.commit(compressed_size);
    pak_buf.consume(compressed_size);

  } else {
    if(compressed) {
      mcd::enc_varint(write_os, packet_size + 1);
      mcd::enc_byte(write_os, 0);
    } else {
      mcd::enc_varint(write_os, packet_size);
    }
    // This copy is stupid and I should refactor it out at some point.
    std::memcpy(write_buf.prepare(packet_size).data(), pak_buf.data().data(),
        packet_size);
    write_buf.commit(packet_size);
    pak_buf.consume(packet_size);
  }

  auto write_size = write_buf.size();
  auto out_mut = out_buf.prepare(write_size);
  std::memcpy(out_mut.data(), write_buf.data().data(), write_size);
  if(encrypted)
    encryptor->process(static_cast<std::uint8_t*>(out_mut.data()), write_size);
  out_buf.commit(write_size);
  write_buf.consume(write_size);

  BOOST_LOG_TRIVIAL(debug) << "Sending packet: " << packet.name;
  ev->emit(packet_event_ids[state][mcd::SERVERBOUND][packet.packet_id],
      static_cast<const void*>(&packet), "mcd::" + packet.name + " *");
}

void IOCore::connect(const std::string& host, const std::string& service) {
  auto endpoints = rslv.resolve(host, service);
  net::async_connect(sock, endpoints, [&](const sys::error_code& ec,
      const ip::tcp::endpoint& ep) {connect_handler(ec, ep);});
}

void IOCore::connect_handler(const sys::error_code& ec,
    const ip::tcp::endpoint& ep) {
  if(ec.failed()) {
    BOOST_LOG_TRIVIAL(fatal) << ec.message();
    exit(-1);
  }

  compressed = false;
  encrypted = false;
  ConnectData data {ep.address().to_string(), ep.port()};
  ev->emit(connect_event, static_cast<const void*>(&data),
      "rkr::ConnectData *");
}

void IOCore::write_handler(const sys::error_code& ec, std::size_t len) {
  if(ec.failed()) {
    BOOST_LOG_TRIVIAL(fatal) << ec.message();
    exit(-1);
  }
  out_buf.consume(len);
}

void IOCore::read_header() {
  int varnum = mcd::verify_varint(
      static_cast<const char *>(in_buf.data().data()), in_buf.size());
  if(varnum == mcd::VARNUM_INVALID) {
    BOOST_LOG_TRIVIAL(fatal) << "Invalid header";
    exit(-1);
  } else if (varnum == mcd::VARNUM_OVERRUN) {
    read_buf = in_buf.prepare(5 - in_buf.size());
    sock.async_read_some(read_buf, [&](const sys::error_code& ec,
        std::size_t len) {header_handler(ec, len);});
  } else {
    auto varint = mcd::dec_varint(in_is);
    if(in_buf.size() >= varint) {
      read_packet(varint);
      return;
    }
    read_buf = in_buf.prepare(varint - in_buf.size());
    net::async_read(sock, read_buf, [&, varint](const sys::error_code& ec,
        std::size_t len) {read_packet_handler(ec, len, varint);});
  }
}

void IOCore::header_handler(const sys::error_code& ec, std::size_t len) {
  if(ec.failed()) {
    BOOST_LOG_TRIVIAL(fatal) << ec.message();
    exit(-1);
  }
  if(encrypted)
    decryptor->process(static_cast<std::uint8_t*>(read_buf.data()), len);
  in_buf.commit(len);
  read_header();
}

void IOCore::read_packet_handler(const sys::error_code& ec, std::size_t len,
    int32_t packet_len) {
  if(ec.failed()) {
    BOOST_LOG_TRIVIAL(fatal) << ec.message();
    exit(-1);
  }
  if(encrypted)
    decryptor->process(static_cast<std::uint8_t*>(read_buf.data()), len);
  in_buf.commit(len);
  read_packet(packet_len);
}

void IOCore::read_packet(size_t len) {
  static std::istream pak_is(&pak_buf);
  std::memcpy(pak_buf.prepare(len).data(), in_buf.data().data(), len);
  pak_buf.commit(len);
  in_buf.consume(len);
  int32_t packet_id;
  if(compressed) {
    auto uncompressed_len = mcd::dec_varint(pak_is);
    if(uncompressed_len) {
      auto remaining_buf = pak_buf.size();
      inflator.next_out = (unsigned char*) pak_buf.prepare(
          uncompressed_len).data();
      inflator.avail_out = uncompressed_len;
      inflator.next_in = (unsigned char *) pak_buf.data().data();
      inflator.avail_in = remaining_buf;
      auto err = inflate(&inflator, Z_FINISH);
      if(err != Z_STREAM_END) {
        BOOST_LOG_TRIVIAL(fatal) << "Err: " << err << " " << inflator.msg;
        exit(-1);
      }
      inflateReset(&inflator);
      pak_buf.commit(uncompressed_len);
      pak_buf.consume(remaining_buf);
    }
  }
  packet_id = mcd::dec_varint(pak_is);
  std::unique_ptr<mcd::Packet> packet;
  try {
    packet = mcd::make_packet(state, mcd::CLIENTBOUND, packet_id);
  } catch(std::exception&) {
    BOOST_LOG_TRIVIAL(fatal) << "Invalid packet id";
    exit(-1);
  }
  packet->decode(pak_is);
  // Needs to be exception based, otherwise reads can cause infinite loops
  if(pak_is.eof() || pak_is.fail() || pak_buf.size()) {
    BOOST_LOG_TRIVIAL(fatal) << "Failed to decode packet, Suspect ID: "
        << packet_id << " Suspect name: " << packet->name;
    BOOST_LOG_TRIVIAL(fatal) << "EOF: " << pak_is.eof() << " Fail: "
        << pak_is.fail() << " Size: " << pak_buf.size();
    exit(-1);
  }
  BOOST_LOG_TRIVIAL(debug) << "Recieved packet: " << packet->name;

  ev->emit(packet_event_ids[state][mcd::CLIENTBOUND][packet->packet_id],
      static_cast<const void*>(packet.get()), "mcd::" + packet->name + " *");
}

void IOCore::encryption_begin_handler(EventCore::ev_id_type ev_id,
    const void* data) {
  auto packet = static_cast<const mcd::ClientboundEncryptionBegin*>(data);
  auto mem = Botan::DataSource_Memory(reinterpret_cast<const std::uint8_t*>(
      packet->publicKey.data()), packet->publicKey.size());
  auto key = Botan::X509::load_key(mem);
  auto& rng = Botan::system_rng();
  auto enc = Botan::PK_Encryptor_EME(*key, rng, "PKCS1v15");
  auto result = enc.encrypt(shared_secret, std::size(shared_secret), rng);
  auto resp = mcd::ServerboundEncryptionBegin();
  resp.sharedSecret.resize(result.size());
  std::memcpy(resp.sharedSecret.data(), result.data(), result.size());
  result = enc.encrypt(reinterpret_cast<const std::uint8_t*>(
      packet->verifyToken.data()), packet->verifyToken.size(), rng);
  resp.verifyToken.resize(result.size());
  std::memcpy(resp.verifyToken.data(), result.data(), result.size());
  encode_packet(resp);
  delete key;
}

void IOCore::enable_encryption(EventCore::ev_id_type ev_id, const void* data) {
  encryptor->clear();
  encryptor->set_key(shared_secret, std::size(shared_secret));
  encryptor->start(shared_secret, std::size(shared_secret));
  decryptor->clear();
  decryptor->set_key(shared_secret, std::size(shared_secret));
  decryptor->start(shared_secret, std::size(shared_secret));
  encrypted = true;
}

void IOCore::enable_compression(EventCore::ev_id_type ev_id,
    const void* data) {
  auto packet = static_cast<const mcd::ClientboundCompress*>(data);
  threshold = packet->threshold;
  compressed = true;
}

void IOCore::transition_state(EventCore::ev_id_type ev_id,
  const void* data) {
  auto packet = static_cast<const mcd::ServerboundSetProtocol*>(data);
  state = static_cast<mcd::packet_state>(packet->nextState);
}

void IOCore::login_success(EventCore::ev_id_type ev_id,
  const void* data) {
  auto packet = static_cast<const mcd::ClientboundSuccess*>(data);
  state = mcd::PLAY;
}

} // namespace rkr
