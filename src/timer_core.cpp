#include "timer_core.hpp"

namespace rkr {

TimerCore::TimerCore(
    rkr::PluginLoader& ploader, net::io_context& ctx, bool ownership)
    : PluginBase("rkr::TimerCore *"), ctx {ctx} {

  ploader.provide("Timer", this, ownership);
}

void TimerCore::register_timer(
    std::function<void()> cb, std::chrono::milliseconds expire) {
  net::steady_timer* timer;
  if(free_timers.empty())
    timer = &timers.emplace_back(ctx);
  else {
    timer = free_timers.top();
    free_timers.pop();
  }
  timer->expires_after(expire);
  timer->async_wait([=, this](const sys::error_code&) {
    cb();
    free_timers.push(timer);
  });
}

} // namespace rkr
