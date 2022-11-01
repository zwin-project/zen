#pragma once

#include <cstdarg>
#include <cstdio>

namespace zen::client {

class Log
{
 public:
  enum Severity {
    DEBUG = 0,  // logs for debugging during development.
    INFO,       // logs that may be useful to some users.
    WARN,       // logs for recoverable failures.
    ERROR,      // logs for unrecoverable failures.
    FATAL,      // logs when aborting.
    SILENT,     // for internal use only.
  };

  static void Print(Severity severity, const char *format, ...)
  {
    switch (severity) {
      case DEBUG:
        fprintf(stderr, "[Debug] ");
        break;
      case INFO:
        fprintf(stderr, "[Info]  ");
        break;
      case WARN:
        fprintf(stderr, "[Warn]  ");
        break;
      case ERROR:
        fprintf(stderr, "[Error] ");
        break;
      case FATAL:
        fprintf(stderr, "[Fatal] ");
        break;
      default:
        break;
    }

    va_list args;
    va_start(args, format);
    std::vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, "\n");
  }
};

#define LOG_DEBUG(format, ...) Log::Print(Log::DEBUG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) Log::Print(Log::INFO, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) Log::Print(Log::WARN, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) Log::Print(Log::ERROR, format, ##__VA_ARGS__)
#define LOG_FATAL(format, ...) Log::Print(Log::FATAL, format, ##__VA_ARGS__)

#define DISABLE_MOVE_AND_COPY(Class)        \
  Class(const Class &) = delete;            \
  Class(Class &&) = delete;                 \
  Class &operator=(const Class &) = delete; \
  Class &operator=(Class &&) = delete

}  // namespace zen::client
