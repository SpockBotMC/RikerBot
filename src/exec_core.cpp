#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

#include "exec_core.hpp"

namespace rkr {

ExecCore::ExecCore(PluginLoader& ploader, bool ownership)
    : PluginBase {"rkr::ExecCore *"} {

  ploader.provide("Exec", this, ownership);
  ev = static_cast<EventCore*>(ploader.require("Event"));
  init_event = ev->register_event("init");
  kill_event = ev->register_event("kill");
}

void ExecCore::run() {
  boost::asio::signal_set signals(ctx, SIGINT, SIGTERM);
  signals.async_wait(
      [&](const sys::error_code& ec, int sig) { signal_handler(ec, sig); });

  ev->emit(init_event);
  ctx.run();
  ev->emit(kill_event);
}

void ExecCore::stop() {
  BOOST_LOG_TRIVIAL(debug) << "Stop called, stopping";
  ctx.stop();
}

net::io_context& ExecCore::get_ctx() {
  return ctx;
}

void ExecCore::signal_handler(const sys::error_code& ec, int sig) {
  if(!ec) {
    BOOST_LOG_TRIVIAL(debug) << "Signal called, stopping";
    ctx.stop();
  } else {
    BOOST_LOG_TRIVIAL(fatal) << "Error in signal handler: " << ec.message();
    exit(-1);
  }
}

} // namespace rkr
