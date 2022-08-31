#ifndef ZEN_OPENVR_BACKEND_OPENVR_H
#define ZEN_OPENVR_BACKEND_OPENVR_H

#include <wayland-server-core.h>

#include <functional>

#include "zen-common.h"

namespace zen {

class OpenVr
{
 public:
  OpenVr() = default;
  ~OpenVr();
  DISABLE_MOVE_AND_COPY(OpenVr)

  struct PollResult {
    bool should_quit;
  };

  bool Init(struct wl_event_loop *loop);

  /** block until connected to OpenVR or failed */
  bool Connect();

  /** Request to poll OpenVR events and HMD poses. `callback` will
   * be called in main thread after the polling finishes. non blocking */
  void RequestPoll(std::function<void(PollResult *result)> callback);

  /** block until swapped */
  // TODO: void Swap();

  /** Finish OpenVR session. The polling callback won't be called after this */
  void Disconnect();

 private:
  void Poll();

  bool connected_ = false;
  std::function<void(PollResult *result)> polling_callback_;
  struct wl_event_source *polling_source_ = NULL;
  int pipe_[2] = {0, 0};
};

}  // namespace zen

#endif  //  ZEN_OPENVR_BACKEND_OPENVR_H
