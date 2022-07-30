#ifndef ZEN_OPENVR_BACKEND_VR_SYSTEM_H
#define ZEN_OPENVR_BACKEND_VR_SYSTEM_H

#include <openvr/openvr.h>
#include <wayland-server-core.h>

#include <functional>
#include <thread>

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
  enum class PollInThreadResult : uint8_t {
    kReadyForNextRepaint = 0,
    kShouldStopOpenVr = 1,
  };

  int HandlePollInThreadResult();
  void PollInThread();
  void StopRepaintLoop();

  bool is_repaint_loop_running_ = false;
  int pipe_[2];  // use pipe_[0] in main thread, pipe_[1] in polling thread
  std::thread poll_thread_;
};

}  // namespace zen

#endif  //  ZEN_OPENVR_BACKEND_VR_SYSTEM_H
