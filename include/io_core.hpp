// I am 31, which is very young for my age.
// That is enough to realize I’m a pencil that has learned
// how to draw the Internet. I explain squiggles
// diagramming exactly how I feel and you are drawn to read
// in ways you cannot yet. Slow goes the drag
// of creation, how what’s within comes to be without,
// which is the rhythmic erection of essence.
//   - Amy King, Wings of Desire


#ifndef IO_CORE_HPP
#define IO_CORE_HPP

#include <string>
#include <cstdint>
#include <memory>
#include <zlib.h>
#include <botan/cipher_mode.h>
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
  int kill;
  std::uint8_t shared_secret[16];

  IOCore(rkr::PluginLoader& ploader, bool ownership = false);
  void run();
  void encode_packet(const mcd::Packet& packet);
  void connect(const std::string& host, const std::string& service);

private:
  EventCore* ev;
  mcd::packet_state state;
  bool compressed;
  bool encrypted;
  net::io_context ctx;
  ip::tcp::socket sock;
  ip::tcp::resolver rslv;
  boost::asio::streambuf out_buf;
  boost::asio::streambuf in_buf;
  boost::asio::streambuf pak_buf;
  std::ostream out_os;
  std::istream in_is;
  EventCore::ev_id_type connect_event;
  EventCore::ev_id_type kill_event;
  std::unique_ptr<Botan::Cipher_Mode> encryptor;
  std::unique_ptr<Botan::Cipher_Mode> decryptor;
  z_stream inflator;
  z_stream deflator;
  std::size_t threshold;
  boost::asio::mutable_buffer read_buf;

  std::array<std::array<std::vector<EventCore::ev_id_type>,
      mcd::DIRECTION_MAX>, mcd::STATE_MAX> packet_event_ids;

  void read_header();
  void read_packet(std::size_t len);
  void connect_handler(const sys::error_code& ec,
      const ip::tcp::endpoint& ep);
  void write_handler(const sys::error_code& ec, std::size_t len);
  void header_handler(const sys::error_code& ec, std::size_t len);
  void read_packet_handler(const sys::error_code& ec, std::size_t len,
      int32_t packet_len);
  void encryption_begin_handler(EventCore::ev_id_type ev_id, const void* data);
  void enable_encryption(EventCore::ev_id_type ev_id, const void* data);
  void enable_compression(EventCore::ev_id_type ev_id, const void* data);
  void transition_state(EventCore::ev_id_type ev_id, const void* data);
  void login_success(EventCore::ev_id_type ev_id, const void* data);
};

} // namespace rkr

#endif
