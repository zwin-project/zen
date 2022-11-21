#include "session.h"

#include <zen-common.h>

static constexpr float kRefreshRate = 60.0f;
static constexpr uint32_t kRefreshIntervalNsec = NSEC_PER_SEC / kRefreshRate;

static int
znr_session_handle_frame_timer(void* data)
{
  auto self = static_cast<znr_session_impl*>(data);

  wl_signal_emit(&self->base.events.frame, nullptr);

  auto now = std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point next = self->prev_frame;

  do {
    next += std::chrono::nanoseconds(kRefreshIntervalNsec);
  } while (now > next);

  std::chrono::nanoseconds duration_nsec = next - now;
  int32_t duration_msec = duration_nsec.count() / 1000000;

  if (duration_msec <= 0) duration_msec = 1;
  wl_event_source_timer_update(self->frame_timer_source, duration_msec);

  self->prev_frame = next;

  return 0;
}

static void
znr_session_handle_disconnect(znr_session_impl* self)
{
  wl_signal_emit(&self->base.events.disconnected, NULL);
}

znr_session_impl*
znr_session_create(std::unique_ptr<zen::remote::server::ISession> proxy,
    struct wl_display* display)
{
  auto self = new znr_session_impl();
  if (self == nullptr) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->proxy = std::move(proxy);
  wl_signal_init(&self->base.events.disconnected);
  wl_signal_init(&self->base.events.frame);
  self->disconnect_signal_connection = self->proxy->on_disconnect.Connect(
      std::bind(znr_session_handle_disconnect, self));

  self->frame_timer_source = wl_event_loop_add_timer(
      wl_display_get_event_loop(display), znr_session_handle_frame_timer, self);

  self->prev_frame = std::chrono::steady_clock::now();

  wl_event_source_timer_update(
      self->frame_timer_source, kRefreshIntervalNsec / 1000000 + 1);

  return self;

err:
  return nullptr;
}

void
znr_session_destroy(znr_session* parent)
{
  struct znr_session_impl* self = zn_container_of(parent, self, base);

  self->disconnect_signal_connection->Disconnect();

  wl_event_source_remove(self->frame_timer_source);
  wl_list_remove(&self->base.events.disconnected.listener_list);
  wl_list_remove(&self->base.events.frame.listener_list);
  delete self;
}
