#ifndef RKR_EXEC_CORE_HPP
#define RKR_EXEC_CORE_HPP

#include <boost/asio/ts/net.hpp>

#ifndef SWIG
#ifdef BOOST_ASIO_TS_NET_HPP
namespace net = boost::asio;
namespace sys = boost::system;
#endif
#endif

#include "event_core.hpp"
#include "plugin_base.hpp"
#include "plugin_loader.hpp"

namespace rkr {

class ExecCore : public PluginBase {
public:
  ExecCore(rkr::PluginLoader& ploader, bool ownership = false);

  void run();
  void stop();
  net::io_context& get_ctx();

private:
  net::io_context ctx;
  EventCore* ev;
  ev_id_type init_event;
  ev_id_type kill_event;

  void signal_handler(const sys::error_code& ec, int sig);
};

} // namespace rkr

#endif // RKR_EXEC_CORE_HPP
