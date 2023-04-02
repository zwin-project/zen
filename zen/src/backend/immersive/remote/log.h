#pragma once

#include <zen-common/log.h>
#include <zen-remote/logger.h>

#include <cstdio>

namespace zen::backend::immersive::remote {

class LogSink : public zen::remote::ILogSink
{
  void Sink(zen::remote::Severity severity, const char * /*pretty_function*/,
      const char *file, int line, const char *format, va_list vp) override
  {
    zn_log_importance_t importance = ZEN_SILENT;
    switch (severity) {
      case zen::remote::Severity::DEBUG:
        importance = ZEN_DEBUG;
        break;
      case zen::remote::Severity::INFO:
        importance = ZEN_INFO;
        break;
      case zen::remote::Severity::WARN:
        importance = ZEN_WARN;
        break;
      case zen::remote::Severity::ERROR:
        importance = ZEN_ERROR;
        break;
      case zen::remote::Severity::FATAL:
        importance = ZEN_ERROR;
        break;
      default:
        break;
    }

    // NOLINTNEXTLINE(cert-err33-c)
    std::snprintf(
        buffer_.data(), buffer_.size(), "[znr] [%s:%d] %s", file, line, format);

    zn_vlog_(importance, buffer_.data(), vp);
  }

  std::array<char, 1024> buffer_;
};

}  // namespace zen::backend::immersive::remote
