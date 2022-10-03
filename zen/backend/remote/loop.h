#pragma once

#include <wayland-server-core.h>
#include <zen/display-system/remote/core/loop.h>

namespace zen::backend::remote {

class RemoteLoop : public display_system::remote::ILoop
{
 public:
  RemoteLoop(struct wl_event_loop* wl_loop) : wl_loop_(wl_loop) {}

  void AddFd(display_system::remote::FdSource* source) override;

  void RemoveFd(display_system::remote::FdSource* source) override;

  void Terminate() override;

 private:
  struct wl_event_loop* wl_loop_;
};

}  // namespace zen::backend::remote
