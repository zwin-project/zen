#pragma once

#include <zen-remote/server/gl-buffer.h>

#include <memory>

#include "znr/gl-buffer.h"

struct znr_gl_buffer_impl {
  znr_gl_buffer base;

  std::unique_ptr<zen::remote::server::IGlBuffer> proxy;
};
