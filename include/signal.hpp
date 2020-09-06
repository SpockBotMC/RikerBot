#ifndef RKR_SIGNAL_HPP
#define RKR_SIGNAL_HPP

#include <csignal>

namespace rkr {

extern volatile std::sig_atomic_t signal_fired;

void set_signal_handlers();

} // namespace rkr

#endif // RKR_SIGNAL_HPP
