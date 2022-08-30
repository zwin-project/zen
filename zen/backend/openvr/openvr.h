#ifndef ZEN_OPENVR_BACKEND_OPENVR_H
#define ZEN_OPENVR_BACKEND_OPENVR_H

#include <wayland-server-core.h>

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

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

  /** start worker thread and block until connected to OpenVR or failed */
  bool Connect();

  /** Request worker thread to poll OpenVR events and HMD poses. `callback` will
   * be called in main thread after the polling finishes. non blocking */
  void RequestPoll(std::function<void(PollResult *result)> callback);

  /** block until swapped */
  // TODO: void Swap();

  /** Finish OpenVR session and block until the worker thread joined */
  void Disconnect();

 private:
  enum class Command {
    kPoll,
    kDisconnect,
  };

  enum class ConnectionStatus {
    kNone,
    kSuccess,
    kFailure,
  };

  enum class Event : uint8_t {
    kPollDone,
  };

  int HandleWorkerEvent();  // call in the main thread

  void HandlePollDone();  // call in the main thread

  void Worker();  // worker thread

  void WorkerPoll();  // call in the worker thread

  void PushCommand(Command command);  // call in the main thread

  Command FetchCommand();  // call in the worker thread

  std::thread worker_thread_;

  // send Event from worker thread (pipe_[1]) to main thread (pipe_[0])
  int pipe_[2];

  std::function<void(PollResult *result)> polling_callback_;

  struct wl_event_source *worker_event_source_ = nullptr;

  struct {
    // pushed by the main thread, consumed by the worker thread
    std::queue<Command> queue;
    std::mutex mutex;
    std::condition_variable cond;
  } command_;

  struct {
    // set by the worker thread, consumed by the main thread
    ConnectionStatus value = ConnectionStatus::kNone;
    std::mutex mutex;
    std::condition_variable cond;
  } connection_status_;

  struct {
    // set by the worker thread, consumed by the main thread
    bool should_quit = false;
    std::mutex mutex;
  } polling_objects_;
};

}  // namespace zen

#endif  //  ZEN_OPENVR_BACKEND_OPENVR_H
