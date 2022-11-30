#pragma once

#include <wayland-server-core.h>
#include <zen-remote/loop.h>

class Loop : public zen::remote::ILoop
{
 public:
  Loop(wl_event_loop* wl_loop) : wl_loop_(wl_loop){};

  void AddFd(zen::remote::FdSource* source) override;

  void RemoveFd(zen::remote::FdSource* source) override;

  void Terminate() override;

 private:
  wl_event_loop* wl_loop_;
};
