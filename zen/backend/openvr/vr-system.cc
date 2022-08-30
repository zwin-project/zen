#include "vr-system.h"

#include <fcntl.h>
#include <openvr/openvr.h>
#include <unistd.h>

namespace zen {

bool
VrSystem::Init(struct wl_event_loop *loop)
{
  return openvr_.Init(loop);
}

bool
VrSystem::Connect()
{
  return openvr_.Connect();
}

void
VrSystem::Disconnect()
{
  openvr_.Disconnect();
  is_repaint_loop_running_ = false;
  if (callbacks.Disconnected) callbacks.Disconnected();
}

void
VrSystem::StartRepaintLoop()
{
  auto callback =
      std::bind(&VrSystem::HandlePollResult, this, std::placeholders::_1);

  if (is_repaint_loop_running_) return;

  openvr_.RequestPoll(callback);

  is_repaint_loop_running_ = true;
}

void
VrSystem::HandlePollResult(OpenVr::PollResult *result)
{
  auto callback =
      std::bind(&VrSystem::HandlePollResult, this, std::placeholders::_1);

  if (is_repaint_loop_running_ == false) return;

  if (result->should_quit) {
    Disconnect();
    return;
  }

  // TODO: render and swap here

  openvr_.RequestPoll(callback);
}

}  // namespace zen
