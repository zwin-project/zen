#include <dbus/dbus.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <systemd/sd-login.h>
#include <unistd.h>
#include <wayland-server.h>
#include <zen-session.h>
#include <zen-util.h>

#include "dbus.h"

struct zn_session_logind {
  struct zn_session base;

  struct wl_display* display;  // non null

  char* session_id;
  unsigned int vt_number;  // 0 for non vt

  DBusConnection* dbus;
  struct wl_event_source* dbus_event_source;
};

/**
 * no side effect
 * @return 0 if equal, < 0 otherwise
 */
static int
seat_id_is_equal_to_session_seat_id(const char* seat_id, char* session_id)
{
  int ret;
  char* session_seat_id = NULL;
  ret = sd_session_get_seat(session_id, &session_seat_id);
  if (ret < 0) {
    zn_log("logind: failed to get session seat\n");
  } else if (strcmp(seat_id, session_seat_id) != 0) {
    zn_log("logind: zen's seat '%s' differs from session-seat '%s'\n", seat_id,
        session_seat_id);
    ret = -EINVAL;
  } else {
    ret = 0;
  }

  free(session_seat_id);

  return ret;
}

ZN_EXPORT int
zn_session_connect(struct zn_session* parent, const char* seat_id)
{
  struct zn_session_logind* self = zn_container_of(parent, self, base);
  struct wl_event_loop* loop;
  int ret;

  ret = sd_pid_get_session(getpid(), &self->session_id);
  if (ret < 0) {
    zn_log("logind: not running in a systemd session\n");
    goto err;
  }

  ret = seat_id_is_equal_to_session_seat_id(seat_id, self->session_id);
  if (ret < 0) goto err_session;

  ret = sd_seat_can_tty(seat_id);
  if (ret > 0) {  // yes
    ret = sd_session_get_vt(self->session_id, &self->vt_number);
    if (ret < 0) {
      zn_log("logind: session not running on a virtual terminal\n");
      goto err_session;
    }
  } else if (ret < 0) {  // unkown
    zn_log(
        "logind: could not determine if seat '%s' has ttys or not\n", seat_id);
    goto err_session;
  }

  self->dbus = zn_dbus_connection_create(DBUS_BUS_SYSTEM);
  if (self->dbus == NULL) goto err_session;

  loop = wl_display_get_event_loop(self->display);

  self->dbus_event_source = zn_dbus_connection_bind(self->dbus, loop);
  if (self->dbus_event_source == NULL) goto err_dbus;

  // TODO: There is more to be done

  return 0;

err_dbus:
  zn_dbus_connection_destroy(self->dbus, self->dbus_event_source);

err_session:
  free(self->session_id);

err:
  return -1;
}

ZN_EXPORT struct zn_session*
zn_session_create(struct wl_display* display)
{
  struct zn_session_logind* self;

  self = zalloc(sizeof *self);
  if (self == NULL) goto err;

  self->display = display;

  return &self->base;

err:
  return NULL;
}

ZN_EXPORT void
zn_session_destroy(struct zn_session* parent)
{
  struct zn_session_logind* self = zn_container_of(parent, self, base);

  zn_dbus_connection_destroy(self->dbus, self->dbus_event_source);
  free(self->session_id);
  free(self);
}
