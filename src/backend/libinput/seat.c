#include <libinput.h>
#include <linux/input-event-codes.h>
#include <stdbool.h>
#include <wayland-server.h>
#include <zen-libinput.h>
#include <zen-session.h>
#include <zen-util.h>

struct zn_libinput {
  struct wl_display *display;
  struct zn_session *session;  // nonnull
  struct libinput *libinput;

  struct wl_event_source *libinput_source;

  bool suspended;
};

static void
libinput_log_func(struct libinput *libinput,
    enum libinput_log_priority priority, const char *format, va_list args)
{
  UNUSED(libinput);
  UNUSED(priority);
  zn_vlog(format, args);
}

static void
zn_libinput_process_events(struct zn_libinput *self)
{
  struct libinput_event *event;

  // FIXME: Here, Q key release is detected and the process is terminated.
  while ((event = libinput_get_event(self->libinput))) {
    if (libinput_event_get_type(event) == LIBINPUT_EVENT_KEYBOARD_KEY) {
      struct libinput_event_keyboard *key_event =
          libinput_event_get_keyboard_event(event);
      enum libinput_key_state key_state =
          libinput_event_keyboard_get_key_state(key_event);
      uint32_t key = libinput_event_keyboard_get_key(key_event);

      if (key_state == LIBINPUT_KEY_STATE_RELEASED && key == KEY_Q)
        wl_display_terminate(self->display);
    }
    libinput_event_destroy(event);
  }
}

static int
zn_libinput_source_dispatch(int fd, uint32_t mask, void *data)
{
  struct zn_libinput *self = data;
  UNUSED(fd);
  UNUSED(mask);

  if (libinput_dispatch(self->libinput) != 0)
    zn_log("libinput: failed to dispatch libinput\n");

  zn_libinput_process_events(self);

  return 0;
}

static int
zn_libinput_open_restricted(const char *path, int flags, void *user_data)
{
  struct zn_libinput *self = user_data;

  return zn_session_open_file(self->session, path, flags);
}

static void
zn_libinput_close_restricted(int fd, void *user_data)
{
  struct zn_libinput *self = user_data;

  zn_session_close_file(self->session, fd);
}

static const struct libinput_interface libinput_interface = {
    .open_restricted = zn_libinput_open_restricted,
    .close_restricted = zn_libinput_close_restricted,
};

static int
zn_libinput_enable(struct zn_libinput *self)
{
  struct wl_event_loop *loop;
  int fd;

  loop = wl_display_get_event_loop(self->display);
  fd = libinput_get_fd(self->libinput);
  self->libinput_source = wl_event_loop_add_fd(
      loop, fd, WL_EVENT_READABLE, zn_libinput_source_dispatch, self);
  if (self->libinput_source == NULL) return -1;

  if (self->suspended) {
    if (libinput_resume(self->libinput) != 0) {
      wl_event_source_remove(self->libinput_source);
      self->libinput_source = NULL;
      return -1;
    }
    self->suspended = true;
    zn_libinput_process_events(self);
  }

  // TODO: notify keyboard focus again

  // TODO: check device existence

  return 0;
}

// TODO: void zn_libinput_disable(struct zn_libinput *self);

ZN_EXPORT struct zn_libinput *
zn_libinput_create(struct udev *udev, struct zn_session *session,
    struct wl_display *display, const char *seat_id)
{
  struct zn_libinput *self;
  enum libinput_log_priority priority = LIBINPUT_LOG_PRIORITY_INFO;

  self = zalloc(sizeof *self);
  if (self == NULL) goto err;

  self->session = session;
  self->display = display;

  self->libinput =
      libinput_udev_create_context(&libinput_interface, self, udev);
  if (self->libinput == NULL) goto err_free;

  libinput_log_set_handler(self->libinput, &libinput_log_func);

  libinput_log_set_priority(self->libinput, priority);

  if (libinput_udev_assign_seat(self->libinput, seat_id) != 0) {
    zn_log("libinput: failed to assign seat id: %s\n", seat_id);
    goto err_unref;
  }

  zn_libinput_process_events(self);

  zn_libinput_enable(self);

  return self;

err_unref:
  libinput_unref(self->libinput);

err_free:
  free(self);

err:
  return NULL;
}

ZN_EXPORT void
zn_libinput_destroy(struct zn_libinput *self)
{
  if (self->libinput_source) wl_event_source_remove(self->libinput_source);
  libinput_unref(self->libinput);
  free(self);
}
