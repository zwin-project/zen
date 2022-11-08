#pragma once

#include <zen-common.h>

#include <functional>
#include <memory>

namespace zen::client {

class Loop;

struct EventSource {
  using Callback = std::function<void(int fd, uint32_t mask, void* data)>;
  int fd;
  int dupfd;
  Callback callback;
  void* data;
  Loop* loop;

  ~EventSource();
};

class Loop
{
 public:
  enum EventMask {
    kEventReadable = 0x01,
    kEventWritable = 0x02,
    kEventHangUp = 0x04,
    kEventError = 0x08,
  };

  DISABLE_MOVE_AND_COPY(Loop);
  Loop() = default;
  ~Loop();

  bool Init();
  int Run();
  void Terminate(int exit_status);

  std::unique_ptr<EventSource> AddFd(
      int fd, uint32_t mask, EventSource::Callback callback, void* data);

 private:
  friend class EventSource;

  bool running_;
  int epoll_fd_ = 0;
  int exit_status_ = EXIT_SUCCESS;
};

}  // namespace zen::client
