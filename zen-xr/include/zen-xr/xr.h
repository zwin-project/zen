#pragma once

#include <wayland-server-core.h>

#include <memory>

namespace zen::xr {

struct System;

struct Xr {
  virtual ~Xr() = default;
};

struct XrDelegate {
  virtual ~XrDelegate() = default;

  virtual void NotifySystemAdded(uint64_t handle) = 0;

  virtual void NotifySystemRemoved(std::unique_ptr<System> &system) = 0;
};

/// @returns nullptr if failed
/// @param delegate can be NULL
std::unique_ptr<Xr> CreateXr(wl_display *display, XrDelegate *delegate);

}  // namespace zen::xr
