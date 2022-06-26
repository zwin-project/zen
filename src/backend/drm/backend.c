#include <zen-backend.h>
#include <zen-session.h>
#include <zen-util.h>

struct zn_drm_backend {
  struct zn_backend base;

  struct zn_session *session;
};

struct zn_backend *
zn_backend_create(struct wl_display *display)
{
  struct zn_drm_backend *self;
  struct zn_session *session;
  const char seat_id[] = "seat0";  // FIXME: enable to configure

  self = zalloc(sizeof *self);
  if (self == NULL) goto err;

  session = zn_session_create(display);
  if (session == NULL) {
    zn_log("drm backend: failed to create a session\n");
    goto err_free;
  }

  if (zn_session_connect(session, seat_id) != 0) {
    zn_log("drm backend: session connection failed\n");
    goto err_session;
  }

  return &self->base;

err_session:
  zn_session_destroy(session);

err_free:
  free(self);

err:
  return NULL;
}

void
zn_backend_destroy(struct zn_backend *parent)
{
  struct zn_drm_backend *self = zn_container_of(parent, self, base);

  zn_session_destroy(self->session);
  free(self);
}
