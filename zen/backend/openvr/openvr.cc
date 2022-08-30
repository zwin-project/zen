#include "openvr.h"

#include <assert.h>
#include <fcntl.h>
#include <openvr/openvr.h>
#include <unistd.h>

namespace zen {

OpenVr::~OpenVr()
{
  if (worker_event_source_) {
    wl_event_source_remove(worker_event_source_);
  }
}

bool
OpenVr::Init(struct wl_event_loop *loop)
{
  if (pipe2(pipe_, O_CLOEXEC) == -1) {
    return false;
  }

  worker_event_source_ = wl_event_loop_add_fd(
      loop, pipe_[0], WL_EVENT_READABLE,
      []([[maybe_unused]] int fd, [[maybe_unused]] uint32_t mask,
          void *data) -> int {
        auto that = static_cast<OpenVr *>(data);
        return that->HandleWorkerEvent();
      },
      this);

  return true;
}

bool
OpenVr::Connect()
{
  std::unique_lock<std::mutex> lock(connection_status_.mutex, std::defer_lock);

  if (worker_thread_.joinable()) {
    Disconnect();
  }

  // no polling thread exist
  connection_status_.value = ConnectionStatus::kNone;

  worker_thread_ = std::thread([this] { this->Worker(); });

  lock.lock();

  connection_status_.cond.wait(lock,
      [this] { return connection_status_.value != ConnectionStatus::kNone; });

  return connection_status_.value == ConnectionStatus::kSuccess;
}

void
OpenVr::RequestPoll(std::function<void(PollResult *result)> callback)
{
  polling_callback_ = callback;
  PushCommand(Command::kPoll);
}

void
OpenVr::Disconnect()
{
  PushCommand(Command::kDisconnect);
  if (worker_thread_.joinable()) worker_thread_.join();

  while (!command_.queue.empty()) {
    command_.queue.pop();
  }
  connection_status_.value = ConnectionStatus::kNone;
  polling_objects_.should_quit = false;
}

int
OpenVr::HandleWorkerEvent()
{
  Event event;
  if (read(pipe_[0], &event, sizeof(event)) != sizeof(event)) {
    zn_abort("Failed to get result of openvr polling thread");
    return 0;
  }

  switch (event) {
    case Event::kPollDone:
      HandlePollDone();
      break;

    default:
      break;
  }

  return 0;
}

void
OpenVr::HandlePollDone()
{
  PollResult result;
  {
    std::lock_guard<std::mutex> lock(polling_objects_.mutex);

    result.should_quit = polling_objects_.should_quit;
  }

  polling_callback_(&result);
}

void
OpenVr::Worker()
{
  Command cmd;
  bool running = true;

  {  // connecting phase
    std::lock_guard<std::mutex> lock(connection_status_.mutex);
    auto init_error = vr::VRInitError_None;
    std::ignore = vr::VR_Init(&init_error, vr::VRApplication_Scene);
    if (init_error != vr::VRInitError_None) {
      connection_status_.value = ConnectionStatus::kFailure;
    } else {
      connection_status_.value = ConnectionStatus::kSuccess;
    }
  }

  connection_status_.cond.notify_one();

  while (running) {
    cmd = FetchCommand();
    switch (cmd) {
      case Command::kPoll:
        WorkerPoll();
        break;

      case Command::kDisconnect:
        running = false;
        break;

      default:
        assert(false && "Unexpected OpenVr::Command");
        running = false;
        break;
    }
  }

  vr::VR_Shutdown();
}

void
OpenVr::WorkerPoll()
{
  vr::TrackedDevicePose_t
      tracked_device_post_list[vr::k_unMaxTrackedDeviceCount];
  vr::VREvent_t vr_event;
  vr::IVRCompositor *compositor = vr::VRCompositor();
  vr::IVRSystem *system = vr::VRSystem();
  bool should_quit = false;
  Event event = Event::kPollDone;

  compositor->WaitGetPoses(
      tracked_device_post_list, vr::k_unMaxTrackedDeviceCount, NULL, 0);

  system->PollNextEvent(&vr_event, sizeof(vr_event));

  switch (vr_event.eventType) {
    case vr::VREvent_Quit:         // fall through
    case vr::VREvent_ProcessQuit:  // fall through
    case vr::VREvent_DriverRequestedQuit:
      should_quit = true;
      break;

    default:
      break;
  }

  {
    std::lock_guard<std::mutex> lock(polling_objects_.mutex);
    polling_objects_.should_quit = should_quit;
  }

  // Nothing we can do if failed
  std::ignore = write(pipe_[1], &event, sizeof(event));
}

void
OpenVr::PushCommand(Command command)
{
  {
    std::lock_guard<std::mutex> lock(command_.mutex);
    command_.queue.push(command);
  }
  command_.cond.notify_one();
}

OpenVr::Command
OpenVr::FetchCommand()
{
  std::unique_lock<std::mutex> lock(command_.mutex);
  Command cmd;

  command_.cond.wait(lock, [this] { return !command_.queue.empty(); });

  cmd = command_.queue.front();
  command_.queue.pop();

  return cmd;
}

}  // namespace zen
