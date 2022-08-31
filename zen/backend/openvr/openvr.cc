#include "openvr.h"

#include <fcntl.h>
#include <openvr/openvr.h>
#include <unistd.h>

namespace zen {

OpenVr::~OpenVr()
{
  if (polling_source_) {
    wl_event_source_remove(polling_source_);
  }

  if (pipe_[0] != 0) close(pipe_[0]);
  if (pipe_[1] != 0) close(pipe_[1]);
}

bool
OpenVr::Init(struct wl_event_loop *loop)
{
  if (pipe2(pipe_, O_CLOEXEC | O_NONBLOCK) == -1) {
    return false;
  }

  polling_source_ = wl_event_loop_add_fd(
      loop, pipe_[0], WL_EVENT_READABLE,
      []([[maybe_unused]] int fd, [[maybe_unused]] uint32_t mask,
          void *data) -> int {
        auto that = static_cast<OpenVr *>(data);
        that->Poll();
        return 0;
      },
      this);

  return true;
}

bool
OpenVr::Connect()
{
  auto init_error = vr::VRInitError_None;

  if (connected_) return true;

  std::ignore = vr::VR_Init(&init_error, vr::VRApplication_Scene);
  if (init_error != vr::VRInitError_None) {
    zn_warn("Failed to init OpenVR: %s",
        vr::VR_GetVRInitErrorAsEnglishDescription(init_error));
  } else {
    connected_ = true;
  }

  return connected_;
}

void
OpenVr::RequestPoll(std::function<void(PollResult *result)> callback)
{
  polling_callback_ = callback;
  char buf = 1;

  // Call this->Poll() in the next wl_event_loop_dispatch
  if (write(pipe_[1], &buf, sizeof(buf)) != sizeof(buf)) {
    zn_error("Failed to write to pipe");
  }
}

void
OpenVr::Disconnect()
{
  connected_ = false;

  vr::VR_Shutdown();
}

void
OpenVr::Poll()
{
  vr::TrackedDevicePose_t
      tracked_device_post_list[vr::k_unMaxTrackedDeviceCount];
  vr::VREvent_t event;
  vr::IVRCompositor *compositor;
  vr::IVRSystem *system;
  PollResult result = {
      false,  // should_quit
  };
  char buf;
  int ret;

  while ((ret = read(pipe_[0], &buf, sizeof(buf))) > 0)
    ;

  if (connected_ == false) return;

  if (ret == 0 || (ret < 0 && errno != EAGAIN)) {
    zn_error("Filed to read from pipe");
    result.should_quit = true;
    goto out;
  }

  compositor = vr::VRCompositor();
  system = vr::VRSystem();

  compositor->WaitGetPoses(
      tracked_device_post_list, vr::k_unMaxTrackedDeviceCount, NULL, 0);

  system->PollNextEvent(&event, sizeof(event));

  switch (event.eventType) {
    case vr::VREvent_Quit:         // fall through
    case vr::VREvent_ProcessQuit:  // fall through
    case vr::VREvent_DriverRequestedQuit:
      result.should_quit = true;
      break;

    default:
      break;
  }

out:
  polling_callback_(&result);
}

}  // namespace zen
