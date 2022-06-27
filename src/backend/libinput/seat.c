#include <libinput.h>
#include <zen-libinput.h>
#include <zen-session.h>
#include <zen-util.h>

struct zn_libinput {
  struct zn_session *session;  // nonnull
  struct libinput *libinput;
};

static void
libinput_log_func(struct libinput *libinput,
    enum libinput_log_priority priority, const char *format, va_list args)
{
  UNUSED(libinput);
  UNUSED(priority);
  zn_vlog(format, args);
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

ZN_EXPORT struct zn_libinput *
zn_libinput_create(
    struct udev *udev, struct zn_session *session, const char *seat_id)
{
  struct zn_libinput *self;
  enum libinput_log_priority priority = LIBINPUT_LOG_PRIORITY_INFO;
  UNUSED(udev);

  self = zalloc(sizeof *self);
  if (self == NULL) goto err;

  self->libinput =
      libinput_udev_create_context(&libinput_interface, self, udev);
  if (self->libinput == NULL) goto err_free;

  libinput_log_set_handler(self->libinput, &libinput_log_func);

  libinput_log_set_priority(self->libinput, priority);

  if (libinput_udev_assign_seat(self->libinput, seat_id) != 0) {
    zn_log("libinput: failed to assign seat id: %s\n", seat_id);
    goto err_unref;
  }

  // TODO: There is more to be done

  self->session = session;

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
  libinput_unref(self->libinput);
  free(self);
}
