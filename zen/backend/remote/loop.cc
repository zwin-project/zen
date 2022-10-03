#include "loop.h"

#include <wayland-server-core.h>
#include <zen/display-system/remote/core/loop.h>

#include "zen-common.h"

namespace zen::backend::remote {

namespace {

int
loop_callback(int fd, uint32_t mask, void* data)
{
  auto source = static_cast<display_system::remote::FdSource*>(data);
  uint32_t remote_mask = 0;

  if (mask & WL_EVENT_READABLE) {
    remote_mask |= display_system::remote::FdSource::kReadable;
  }
  if (mask & WL_EVENT_WRITABLE) {
    remote_mask |= display_system::remote::FdSource::kWritable;
  }
  if (mask & WL_EVENT_HANGUP) {
    remote_mask |= display_system::remote::FdSource::kHangup;
  }
  if (mask & WL_EVENT_ERROR) {
    remote_mask |= display_system::remote::FdSource::kError;
  }

  auto c = source->callback;
  source->callback(fd, remote_mask);

  return 1;
}

}  // namespace

void
RemoteLoop::AddFd(display_system::remote::FdSource* source)
{
  uint32_t mask = 0;

  if (source->mask & display_system::remote::FdSource::kReadable) {
    mask |= WL_EVENT_READABLE;
  }
  if (source->mask & display_system::remote::FdSource::kWritable) {
    mask |= WL_EVENT_WRITABLE;
  }
  if (source->mask & display_system::remote::FdSource::kHangup) {
    mask |= WL_EVENT_HANGUP;
  }
  if (source->mask & display_system::remote::FdSource::kError) {
    mask |= WL_EVENT_ERROR;
  }

  source->data =
      wl_event_loop_add_fd(wl_loop_, source->fd, mask, loop_callback, source);
}

void
RemoteLoop::RemoveFd(display_system::remote::FdSource* source)
{
  wl_event_source_remove((wl_event_source*)source->data);
}

void
RemoteLoop::Terminate()
{
  zn_terminate(EXIT_FAILURE);
}

}  // namespace zen::backend::remote
