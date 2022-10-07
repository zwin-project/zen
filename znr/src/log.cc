#include "znr/log.h"

#include <zen-common.h>
#include <zen-remote/logger.h>

#include <memory>
#include <string>

class LogSink : public zen::remote::ILogSink
{
  void Sink(zen::remote::Severity severity, const char* /*pretty_function*/,
      const char* file, int line, const char* format, va_list vp) override
  {
    static char zn_fmt[1024] = {0};

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

    snprintf(zn_fmt, sizeof(zn_fmt), "[znr] [%s:%d] %s", file, line, format);
    _zn_vlog(importance, zn_fmt, vp);
  }
};

void
znr_log_init(void)
{
  zen::remote::InitializeLogger(std::make_unique<LogSink>());
}
