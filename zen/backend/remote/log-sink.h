#pragma once

#include <zen/display-system/remote/core/logger.h>

#include "zen-common.h"

namespace zen::backend::remote {

class RemoteLogSink : public zen::display_system::remote::log::ILogSink
{
  void Sink(zen::display_system::remote::log::Severity severity,
      const char* pretty_function, const char* file, int line,
      const char* format, va_list vp) override;
};

}  // namespace zen::backend::remote
