#include "loop.h"

#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <functional>

namespace zen::client {

bool
Loop::Init()
{
  epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
  if (epoll_fd_ == -1) {
    LOG_ERROR("Failed to create epoll instance");
    return false;
  }
  return true;
}

int
Loop::Run()
{
  int epoll_count;
  struct epoll_event events[16];
  EventSource* source;

  running_ = true;
  while (running_) {
    epoll_count = epoll_wait(epoll_fd_, events, 16, -1);
    for (int i = 0; i < epoll_count; i++) {
      source = static_cast<EventSource*>(events[i].data.ptr);
      uint32_t mask = 0;
      if (events->events & EPOLLIN) mask |= kEventReadable;
      if (events->events & EPOLLOUT) mask |= kEventWritable;
      if (events->events & EPOLLHUP) mask |= kEventHangUp;
      if (events->events & EPOLLERR) mask |= kEventError;
      source->callback(source->fd, mask, source->data);
    }
  }

  return exit_status_;
}

void
Loop::Terminate(int exit_status)
{
  exit_status_ = exit_status;
  running_ = false;  // TODO: wakeup epoll instance
}

Loop::~Loop()
{
  if (epoll_fd_ > 0) close(epoll_fd_);
}

std::unique_ptr<EventSource>
Loop::AddFd(int fd, uint32_t mask, EventSource::Callback callback, void* data)
{
  struct epoll_event ep;
  auto source = new EventSource();

  // TODO: fallback for older linux
  source->dupfd = fcntl(fd, F_DUPFD_CLOEXEC, 0);

  source->fd = fd;
  source->data = data;
  source->callback = callback;
  source->loop = this;

  if (mask & kEventReadable) ep.events |= EPOLLIN;
  if (mask & kEventWritable) ep.events |= EPOLLOUT;
  ep.data.ptr = source;

  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, source->dupfd, &ep) < 0) {
    close(source->dupfd);
    delete source;
    return std::unique_ptr<EventSource>();
  }

  return std::unique_ptr<EventSource>(source);
}

EventSource::~EventSource()
{
  epoll_ctl(loop->epoll_fd_, EPOLL_CTL_DEL, dupfd, NULL);
  close(dupfd);
}

}  // namespace zen::client
