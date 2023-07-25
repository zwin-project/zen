#include "loop.hh"

#include "zen-common/terminate.h"

namespace zen::backend::immersive::remote {

namespace {

int
loop_callback(int fd, uint32_t mask, void *data)
{
  auto *source = static_cast<zen::remote::FdSource *>(data);

  uint32_t remote_mask = 0;

  if ((mask & WL_EVENT_READABLE) == WL_EVENT_READABLE) {
    remote_mask |= zen::remote::FdSource::kReadable;
  }

  if ((mask & WL_EVENT_WRITABLE) == WL_EVENT_WRITABLE) {
    remote_mask |= zen::remote::FdSource::kWritable;
  }

  if ((mask & WL_EVENT_HANGUP) == WL_EVENT_HANGUP) {
    remote_mask |= zen::remote::FdSource::kHangup;
  }

  if ((mask & WL_EVENT_ERROR) == WL_EVENT_ERROR) {
    remote_mask |= zen::remote::FdSource::kError;
  }

  source->callback(fd, remote_mask);

  return 1;
}

}  // namespace

Loop::Loop(wl_event_loop *wl_loop) : wl_loop_(wl_loop) {}

void
Loop::AddFd(zen::remote::FdSource *source)
{
  using FdSource = zen::remote::FdSource;

  uint32_t mask = 0;

  if ((source->mask & FdSource::kReadable) == FdSource::kReadable) {
    mask |= WL_EVENT_READABLE;
  }
  if ((source->mask & FdSource::kWritable) == FdSource::kWritable) {
    mask |= WL_EVENT_WRITABLE;
  }
  if ((source->mask & FdSource::kHangup) == FdSource::kHangup) {
    mask |= WL_EVENT_HANGUP;
  }
  if ((source->mask & FdSource::kError) == FdSource::kError) {
    mask |= WL_EVENT_ERROR;
  }

  source->data =
      wl_event_loop_add_fd(wl_loop_, source->fd, mask, loop_callback, source);
}

void
Loop::RemoveFd(zen::remote::FdSource *source)
{
  wl_event_source_remove(static_cast<wl_event_source *>(source->data));
}

void
Loop::Terminate()
{
  zn_terminate(EXIT_FAILURE);
}

}  // namespace zen::backend::immersive::remote
