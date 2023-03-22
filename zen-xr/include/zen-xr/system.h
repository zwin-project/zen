#pragma once

#include <memory>

namespace zen::xr {

struct System {
  enum Type {
    kNone,
    kRemote,
  };

  virtual ~System() = default;

  virtual uint64_t handle() = 0;
  virtual Type type() = 0;
};

}  // namespace zen::xr
