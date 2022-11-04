#include "loop.h"

#include <zen-common.h>

namespace {

int
loop_callback(int fd, uint32_t mask, void* data)
{
  auto source = static_cast<zen::remote::FdSource*>(data);
  uint32_t remote_mask = 0;

  if (mask & WL_EVENT_READABLE) {
    remote_mask |= zen::remote::FdSource::kReadable;
  }
  if (mask & WL_EVENT_WRITABLE) {
    remote_mask |= zen::remote::FdSource::kWritable;
  }
  if (mask & WL_EVENT_HANGUP) {
    remote_mask |= zen::remote::FdSource::kHangup;
  }
  if (mask & WL_EVENT_ERROR) {
    remote_mask |= zen::remote::FdSource::kError;
  }

  auto c = source->callback;
  source->callback(fd, remote_mask);

  return 1;
}

}  // namespace

void
Loop::AddFd(zen::remote::FdSource* source)
{
  uint32_t mask = 0;

  if (source->mask & zen::remote::FdSource::kReadable) {
    mask |= WL_EVENT_READABLE;
  }
  if (source->mask & zen::remote::FdSource::kWritable) {
    mask |= WL_EVENT_WRITABLE;
  }
  if (source->mask & zen::remote::FdSource::kHangup) {
    mask |= WL_EVENT_HANGUP;
  }
  if (source->mask & zen::remote::FdSource::kError) {
    mask |= WL_EVENT_ERROR;
  }

  source->data =
      wl_event_loop_add_fd(wl_loop_, source->fd, mask, loop_callback, source);
}

void
Loop::RemoveFd(zen::remote::FdSource* source)
{
  wl_event_source_remove((wl_event_source*)source->data);
}

void
Loop::Terminate()
{
  zn_terminate(EXIT_FAILURE);
}
