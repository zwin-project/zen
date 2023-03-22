#pragma once

#include "common.h"
#include "xr.h"

namespace zen::xr::proxy {

class XrSystemProxy;

class XrProxy : public zen::xr::XrDelegate
{
 public:
  DISABLE_MOVE_AND_COPY(XrProxy);
  ~XrProxy() override;

  void NotifySystemAdded(uint64_t handle) override;

  void NotifySystemRemoved(std::unique_ptr<zen::xr::System> &system) override;

 private:
  friend zn_xr * ::zn_xr_create(struct wl_display *display);

  XrProxy();

  bool Init(wl_display *display);

  std::unique_ptr<zen::xr::Xr> xr_;

  zn_xr zn_xr_{};

  std::vector<std::unique_ptr<XrSystemProxy>> systems_;
};

}  // namespace zen::xr::proxy
