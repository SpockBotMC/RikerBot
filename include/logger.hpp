/*
  This header and its associated implementation are thin (very thin) wrappers
  around boost.log, and they're provided to make the logger available to Python
  plugins. C++ plugins should just use boost.log directly.
*/

#ifndef RKR_LOGGER_HPP
#define RKR_LOGGER_HPP

#include <string>

namespace rkr {

enum severity_level {
  level_trace,
  level_debug,
  level_info,
  level_warning,
  level_error,
  level_fatal
};

void set_log_level(const severity_level sev);

#define LOG_FUNC(sev) void sev(const std::string& s);
LOG_FUNC(trace)
LOG_FUNC(debug)
LOG_FUNC(info)
LOG_FUNC(warning)
LOG_FUNC(error)
LOG_FUNC(fatal)
#undef LOG_FUNC

} // namespace rkr
#endif // RKR_LOGGER_HPP
