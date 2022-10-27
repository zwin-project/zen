#pragma once

#include <zen-remote/server/buffer.h>

#include <memory>

#include "znr/buffer.h"

struct znr_buffer_impl {
  znr_buffer base;

  struct znr_remote_impl* remote;
  void* data;
  uint32_t ref;
};

std::unique_ptr<zen::remote::server::IBuffer> znr_buffer_impl_use(
    struct znr_buffer_impl* self);
