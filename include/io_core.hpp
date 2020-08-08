#ifndef IO_CORE_HPP
#define IO_CORE_HPP

#include <string>
#include <cstdint>
#include <memory>
#include <botan/cipher_mode.h>
#include <botan/compression.h>
#include <net.hpp>
#include <boost/asio.hpp>

// I have no idea why SWIG hates namespace aliasing, but what it can't see
// won't hurt it.
#ifndef SWIG
#ifdef BOOST_ASIO_TS_NET_HPP
namespace net = boost::asio;
namespace sys = boost::system;
#endif
namespace ip = net::ip;
#endif

#include "plugin_loader.hpp"
#include "plugin_base.hpp"
#include "event_core.hpp"
#include "minecraft_protocol.hpp"


namespace rkr {

struct ConnectData {
  std::string host;
  std::uint16_t port;
};

class IOCore : public PluginBase {
public:
  mcd::packet_state state;
  int kill;
  std::uint8_t shared_secret[16];

  IOCore(rkr::PluginLoader& ploader, bool ownership = false);
  void run();
  void encode_packet(const mcd::Packet& packet);
  void connect(const std::string& host, const std::string& service);

private:
  EventCore* ev;
  bool compressed;
  bool encrypted;
  net::io_context ctx;
  ip::tcp::socket sock;
  ip::tcp::resolver rslv;
  boost::asio::streambuf out_buf;
  boost::asio::streambuf in_buf;
  std::ostream out_os;
  std::istream in_is;
  EventCore::ev_id_type connect_event;
  EventCore::ev_id_type kill_event;
  std::unique_ptr<Botan::Cipher_Mode> encryptor;
  std::unique_ptr<Botan::Cipher_Mode> decryptor;
  std::unique_ptr<Botan::Compression_Algorithm> compressor;
  std::unique_ptr<Botan::Decompression_Algorithm> decompressor;
  std::size_t threshold;
  boost::asio::mutable_buffer read_buf;

  std::array<std::array<std::vector<EventCore::ev_id_type>,
      mcd::DIRECTION_MAX>, mcd::STATE_MAX> packet_event_ids;

  void connect_handler(const sys::error_code& ec,
      const ip::tcp::endpoint& ep);
  void write_handler(const sys::error_code& ec, std::size_t len);
  void header_handler(const sys::error_code& ec, std::size_t len);
  void read_packet_handler(const sys::error_code& ec, std::size_t len);
  void encryption_begin_handler(EventCore::ev_id_type ev_id, const void* data);
  void enable_encryption(EventCore::ev_id_type ev_id, const void* data);
  void enable_compression(EventCore::ev_id_type ev_id, const void* data);
};

} // namespace rkr

#endif
