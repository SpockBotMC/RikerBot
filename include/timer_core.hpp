#ifndef RKR_TIMER_CORE_HPP
#define RKR_TIMER_CORE_HPP

#include <chrono>
#include <cstdint>
#include <functional>
#include <stack>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <boost/asio.hpp>
#include <boost/asio/ts/net.hpp>

#include "event_core.hpp"
#include "plugin_base.hpp"
#include "plugin_loader.hpp"

#ifndef SWIG
#ifdef BOOST_ASIO_TS_NET_HPP
namespace net = boost::asio;
namespace sys = boost::system;
#endif
#endif

namespace rkr {

class TimerCore : public PluginBase {
public:
  TimerCore(rkr::PluginLoader& ploader, net::io_context& ctx,
      bool ownership = false);

  void register_timer(
      std::function<void()> cb, std::chrono::milliseconds expire);

private:
  net::io_context& ctx;
  std::vector<net::steady_timer> timers;
  std::stack<net::steady_timer*> free_timers;
};

} // namespace rkr

#endif // RKR_TIMER_CORE_HPP
