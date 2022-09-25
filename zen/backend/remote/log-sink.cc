#include "log-sink.h"

#include <iostream>
#include <sstream>

namespace zen::backend::remote {

namespace remote = zen::display_system::remote;

void
RemoteLogSink::Sink(remote::log::Severity severity,
    const char* /*pretty_function*/, const char* file, int line,
    const char* format, va_list vp)
{
  zn_log_importance_t importance = ZEN_INFO;
  switch (severity) {
    case remote::log::DEBUG:
      importance = ZEN_DEBUG;
      break;
    case remote::log::INFO:
      importance = ZEN_INFO;
      break;
    case remote::log::WARN:
      importance = ZEN_WARN;
      break;
    case remote::log::ERROR:  // fall through
    case remote::log::FATAL:
      importance = ZEN_ERROR;
      break;

    default:
      break;
  }
  std::ostringstream full_format;
  full_format << "[zen remote] [" << file << ":" << line << "] ";
  full_format << format;
  _zn_vlog(importance, full_format.str().c_str(), vp);
}

}  // namespace zen::backend::remote
