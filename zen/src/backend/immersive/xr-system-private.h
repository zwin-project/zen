#pragma once

#include "common.h"
#include "zen/xr-system.h"

namespace zen::xr::proxy {

class XrSystemProxy
{
 public:
  DISABLE_MOVE_AND_COPY(XrSystemProxy);
  explicit XrSystemProxy(uint64_t handle);
  ~XrSystemProxy();

  bool Init();

  inline uint64_t handle() const;
  inline zn_xr_system *zn_xr_system();

 private:
  struct zn_xr_system zn_xr_system_ {};
};

inline uint64_t
XrSystemProxy::handle() const
{
  return zn_xr_system_.handle;
}

inline zn_xr_system *
XrSystemProxy::zn_xr_system()
{
  return &zn_xr_system_;
}

}  // namespace zen::xr::proxy
