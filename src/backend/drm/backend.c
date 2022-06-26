#include <libudev.h>
#include <zen-backend.h>
#include <zen-libinput.h>
#include <zen-session.h>
#include <zen-util.h>

struct zn_drm_backend {
  struct zn_backend base;

  struct zn_session *session;
  struct udev *udev;
  struct zn_libinput *input;
};

ZN_EXPORT struct zn_backend *
zn_backend_create(struct wl_display *display)
{
  struct zn_drm_backend *self;
  const char seat_id[] = "seat0";  // FIXME: enable to configure

  self = zalloc(sizeof *self);
  if (self == NULL) goto err;

  self->session = zn_session_create(display);
  if (self->session == NULL) {
    zn_log("drm backend: failed to create a session\n");
    goto err_free;
  }

  if (zn_session_connect(self->session, seat_id) != 0) {
    zn_log("drm backend: session connection failed\n");
    goto err_session;
  }

  self->udev = udev_new();
  if (self->udev == NULL) {
    zn_log("drm backend: failed to initialize udev context\n");
    goto err_session;
  }

  self->input = zn_libinput_create(self->udev, self->session);
  if (self->input == NULL) {
    zn_log("drm backend: failed to create input devices\n");
    goto err_udev;
  }

  return &self->base;

err_udev:
  udev_unref(self->udev);

err_session:
  zn_session_destroy(self->session);

err_free:
  free(self);

err:
  return NULL;
}

ZN_EXPORT void
zn_backend_destroy(struct zn_backend *parent)
{
  struct zn_drm_backend *self = zn_container_of(parent, self, base);

  zn_libinput_destroy(self->input);
  udev_unref(self->udev);
  zn_session_destroy(self->session);
  free(self);
}
