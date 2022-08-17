#include "vr-system.h"

#include <fcntl.h>
#include <openvr/openvr.h>
#include <unistd.h>

namespace zen {

bool
VrSystem::Init(struct wl_event_loop *loop)
{
  if (pipe2(pipe_, O_CLOEXEC) == -1) return false;

  wl_event_loop_add_fd(
      loop, pipe_[0], WL_EVENT_READABLE,
      []([[maybe_unused]] int fd, [[maybe_unused]] uint32_t mask,
          void *data) -> int {
        auto that = static_cast<VrSystem *>(data);
        return that->HandlePollInThreadResult();
      },
      this);

  return true;
}

bool
VrSystem::Connect()
{
  auto init_error = vr::VRInitError_None;

  if (poll_thread_.joinable()) poll_thread_.join();
  std::ignore = vr::VR_Init(&init_error, vr::VRApplication_Scene);
  if (init_error != vr::VRInitError_None) {
    zn_warn("Failed to init OpenVR: %s",
        vr::VR_GetVRInitErrorAsEnglishDescription(init_error));
    goto err;
  }

  return true;

err:
  return false;
}

void
VrSystem::Disconnect()
{
  this->StopRepaintLoop();
  if (poll_thread_.joinable()) poll_thread_.join();
  vr::VR_Shutdown();
  if (callbacks.Disconnected) callbacks.Disconnected();
}

int
VrSystem::HandlePollInThreadResult()
{
  VrSystem::PollInThreadResult res;

  if (read(pipe_[0], &res, sizeof(res)) != sizeof(res)) {
    zn_abort("Failed to get result of openvr polling thread");
    return 0;
  }

  switch (res) {
    case VrSystem::PollInThreadResult::kShouldStopOpenVr:
      this->Disconnect();
      break;

    case VrSystem::PollInThreadResult::kReadyForNextRepaint:
      // TODO: render and IVRCompositor::Submit here
      if (is_repaint_loop_running_) this->PollInThread();
      break;
    default:
      break;
  }

  return 0;
}

void
VrSystem::PollInThread()
{
  if (poll_thread_.joinable()) poll_thread_.join();
  poll_thread_ = std::thread([this]() {
    vr::TrackedDevicePose_t
        tracked_device_post_list[vr::k_unMaxTrackedDeviceCount];
    vr::VREvent_t event;
    vr::IVRCompositor *compositor = vr::VRCompositor();
    vr::IVRSystem *system = vr::VRSystem();
    VrSystem::PollInThreadResult res;

    compositor->WaitGetPoses(
        tracked_device_post_list, vr::k_unMaxTrackedDeviceCount, NULL, 0);

    system->PollNextEvent(&event, sizeof(event));

    switch (event.eventType) {
      case vr::VREvent_Quit:         // fall through
      case vr::VREvent_ProcessQuit:  // fall through
      case vr::VREvent_DriverRequestedQuit:
        res = VrSystem::PollInThreadResult::kShouldStopOpenVr;

        // just return even if write fails
        std::ignore =
            write(pipe_[1], &res, sizeof(VrSystem::PollInThreadResult));
        return;

      default:
        break;
    }

    res = VrSystem::PollInThreadResult::kReadyForNextRepaint;
    std::ignore = write(pipe_[1], &res, sizeof(VrSystem::PollInThreadResult));
  });
}

void
VrSystem::StartRepaintLoop()
{
  if (is_repaint_loop_running_) return;
  is_repaint_loop_running_ = true;
  this->PollInThread();
}

void
VrSystem::StopRepaintLoop()
{
  is_repaint_loop_running_ = false;
}

}  // namespace zen
