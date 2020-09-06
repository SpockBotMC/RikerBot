#include <csignal>

namespace rkr {

volatile std::sig_atomic_t signal_fired = 0;

void signal_handler(int sig) {
  signal_fired = 1;
}

void set_signal_handlers() {
  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);
}

} // namespace rkr
