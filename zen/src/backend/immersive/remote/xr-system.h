#pragma once

#include "zen-common/cpp-util.h"
#include "zen/xr-system.h"

namespace zen::backend::immersive::remote {

class XrSystem
{
 public:
  DISABLE_MOVE_AND_COPY(XrSystem);
  ~XrSystem();

  static std::unique_ptr<XrSystem> New(uint64_t peer_id);

  inline uint64_t peer_id() const;

  inline zn_xr_system *c_obj();

 private:
  explicit XrSystem(uint64_t peer_id);

  bool Init();

  const uint64_t peer_id_;

  zn_xr_system c_obj_{};
};

inline uint64_t
XrSystem::peer_id() const
{
  return peer_id_;
}

inline zn_xr_system *
XrSystem::c_obj()
{
  return &c_obj_;
}

}  // namespace zen::backend::immersive::remote
