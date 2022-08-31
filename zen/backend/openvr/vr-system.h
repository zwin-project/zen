#ifndef ZEN_OPENVR_BACKEND_VR_SYSTEM_H
#define ZEN_OPENVR_BACKEND_VR_SYSTEM_H

#include <wayland-server-core.h>

#include <functional>

#include "openvr.h"
#include "zen-common.h"

namespace zen {

class VrSystem
{
 public:
  VrSystem() = default;
  ~VrSystem() = default;
  DISABLE_MOVE_AND_COPY(VrSystem)

  bool Init(struct wl_event_loop *loop);
  bool Connect();
  void Disconnect();
  void StartRepaintLoop();

  struct {
    std::function<void()> Disconnected;
  } callbacks;

 private:
  void HandlePollResult(OpenVr::PollResult *result);

  OpenVr openvr_;
  bool is_repaint_loop_running_ = false;
};

}  // namespace zen

#endif  //  ZEN_OPENVR_BACKEND_VR_SYSTEM_H
