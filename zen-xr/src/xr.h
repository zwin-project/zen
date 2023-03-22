#pragma once

#include "common.h"
#include "zen-xr/xr.h"

namespace zen::xr {

struct System;

class XrImpl : public Xr
{
 public:
  DISABLE_MOVE_AND_COPY(XrImpl);
  ~XrImpl() override;

 private:
  friend std::unique_ptr<Xr> CreateXr(
      wl_display *display, XrDelegate *delegate);

  XrImpl(wl_display *display, XrDelegate *delegate);

  void RemoveSystemIf(std::function<bool(std::unique_ptr<System> &)> pred);

  void HandleRemotePeerDiscover(uint64_t id);

  void HandleRemotePeerLost(uint64_t id);

  bool Init();

  /// @return value must not be 0
  uint64_t NextHandle();

  std::vector<std::unique_ptr<System>> systems_;

  std::unique_ptr<zen::remote::server::IPeerManager> peer_manager_;  // @nonnull

  wl_display *display_;  // @nonnull, @outlive

  XrDelegate *delegate_;  // @nullable, @outlive if exists

  uint64_t next_handle_ = 0;
};

}  // namespace zen::xr
