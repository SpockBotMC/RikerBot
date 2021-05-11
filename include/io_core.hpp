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

#include <boost/asio.hpp>
#include <boost/asio/ts/net.hpp>
#include <botan/cipher_mode.h>
#include <cstdint>
#include <memory>
#include <string>
#include <zlib.h>

// I have no idea why SWIG hates namespace aliasing, but what it can't see
// won't hurt it.
#ifndef SWIG
#ifdef BOOST_ASIO_TS_NET_HPP
namespace net = boost::asio;
namespace sys = boost::system;
#endif
namespace ip = net::ip;
#endif

#include "event_core.hpp"
#include "minecraft_protocol.hpp"
#include "plugin_base.hpp"
#include "plugin_loader.hpp"

namespace rkr {

struct ConnectData {
  std::string host;
  std::uint16_t port;
};

class IOCore : public PluginBase {
public:
  std::uint8_t shared_secret[16];

  IOCore(
      rkr::PluginLoader& ploader, net::io_context& ctx, bool ownership = false);
  void encode_packet(const mcd::Packet& packet);
  void connect(const std::string& host, const std::string& service);

private:
  EventCore* ev;
  mcd::packet_state state {mcd::HANDSHAKING};
  bool compressed {false};
  bool encrypted {false};
  bool ongoing_write {false};
  ip::tcp::socket sock;
  ip::tcp::resolver rslv;
  boost::asio::steady_timer tick_timer;

  std::deque<boost::asio::streambuf> write_bufs;
  std::deque<boost::asio::const_buffer> out_bufs;

  boost::asio::mutable_buffer in_buf;
  boost::asio::streambuf read_buf;
  std::istream read_is {&read_buf};

  ev_id_type connect_event;
  ev_id_type kill_event;
  ev_id_type tick_event;

  std::unique_ptr<Botan::Cipher_Mode> encryptor {
      Botan::Cipher_Mode::create("AES-128/CFB/8", Botan::ENCRYPTION)};
  std::unique_ptr<Botan::Cipher_Mode> decryptor {
      Botan::Cipher_Mode::create("AES-128/CFB/8", Botan::DECRYPTION)};

  z_stream inflator {};
  z_stream deflator {};
  std::size_t threshold;

  std::array<std::array<std::vector<ev_id_type>, mcd::DIRECTION_MAX>,
      mcd::STATE_MAX>
      packet_event_ids;

  void tick();
  void read_packet();
  void write_packet(
      const boost::asio::streambuf& header, const boost::asio::streambuf& body);
  void read_header();
  void read_body(std::size_t len);
  void connect_handler(const sys::error_code& ec, const ip::tcp::endpoint& ep);
  void write_handler(const sys::error_code& ec, std::size_t len);
  void header_handler(const sys::error_code& ec, std::size_t len);
  void body_handler(
      const sys::error_code& ec, std::size_t len, int32_t body_len);
  void encryption_begin_handler(const void* data);
  void enable_encryption(const void* data);
  void enable_compression(const void* data);
  void transition_state(const void* data);
  void login_success(const void* data);
};

} // namespace rkr

#endif
