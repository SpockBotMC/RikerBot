#include "logger.hpp"
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>

namespace rkr {

void set_log_level(const severity_level sev) {
  auto x {boost::log::core::get()};
  switch(sev) {
  case level_trace:
    x->set_filter(boost::log::trivial::severity >= boost::log::trivial::trace);
    break;
  case level_debug:
    x->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);
    break;
  case level_info:
    x->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
    break;
  case level_warning:
    x->set_filter(
        boost::log::trivial::severity >= boost::log::trivial::warning);
    break;
  case level_error:
    x->set_filter(boost::log::trivial::severity >= boost::log::trivial::error);
    break;
  case level_fatal:
    x->set_filter(boost::log::trivial::severity >= boost::log::trivial::fatal);
    break;
  }
}

#define LOG_FUNC(sev)                                                          \
  void sev(const std::string& s) {                                             \
    BOOST_LOG_TRIVIAL(sev) << s;                                               \
  }
LOG_FUNC(trace)
LOG_FUNC(debug)
LOG_FUNC(info)
LOG_FUNC(warning)
LOG_FUNC(error)
LOG_FUNC(fatal)
#undef LOG_FUNC

} // namespace rkr
