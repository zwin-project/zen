#include "dbus.h"

#include <assert.h>
#include <errno.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <zen-util.h>

static int
zn_dbus_connection_dispatch(int fd, uint32_t mask, void* data)
{
  DBusConnection* connection = data;
  DBusDispatchStatus status;
  (void)fd;
  (void)mask;

  do {
    status = dbus_connection_dispatch(connection);
    switch (status) {
      case DBUS_DISPATCH_COMPLETE:
        return 0;

      case DBUS_DISPATCH_DATA_REMAINS:
        break;

      case DBUS_DISPATCH_NEED_MEMORY:
        zn_log("dbus: cannot dispatch dbus event: no memory\n");
        return 0;

      default:
        zn_log("dbus: cannot dispatch dbus event\n");
        return 0;
    }
  } while (true);
}

static int
zn_dbus_connection_dispatch_watch(int fd, uint32_t mask, void* data)
{
  DBusWatch* watch = data;
  uint32_t flags = 0;
  (void)fd;

  if (dbus_watch_get_enabled(watch)) {
    if (mask & WL_EVENT_READABLE) flags |= DBUS_WATCH_READABLE;
    if (mask & WL_EVENT_WRITABLE) flags |= DBUS_WATCH_WRITABLE;
    if (mask & WL_EVENT_HANGUP) flags |= DBUS_WATCH_HANGUP;
    if (mask & WL_EVENT_ERROR) flags |= DBUS_WATCH_ERROR;

    dbus_watch_handle(watch, flags);
  }

  return 0;
}

static dbus_bool_t
zn_dbus_connection_add_watch(DBusWatch* watch, void* data)
{
  struct wl_event_loop* loop = data;
  struct wl_event_source* event_source;
  int fd;
  uint32_t mask = 0, flags;

  if (dbus_watch_get_enabled(watch)) {
    flags = dbus_watch_get_flags(watch);
    if (flags & DBUS_WATCH_READABLE) mask |= WL_EVENT_READABLE;
    if (flags & DBUS_WATCH_WRITABLE) mask |= WL_EVENT_WRITABLE;
  }

  fd = dbus_watch_get_unix_fd(watch);
  event_source = wl_event_loop_add_fd(
      loop, fd, mask, zn_dbus_connection_dispatch_watch, watch);

  if (event_source == NULL) return FALSE;

  dbus_watch_set_data(watch, event_source, NULL);

  return TRUE;
}

static void
zn_dbus_connection_remove_watch(DBusWatch* watch, void* data)
{
  struct wl_event_source* event_source;
  (void)data;

  event_source = dbus_watch_get_data(watch);
  if (event_source == NULL) return;

  wl_event_source_remove(event_source);
}

static void
zn_dbus_connection_toggle_watch(DBusWatch* watch, void* data)
{
  struct wl_event_source* event_source;
  uint32_t mask = 0, flags;
  (void)watch;
  (void)data;

  event_source = dbus_watch_get_data(watch);
  if (!event_source) return;

  if (dbus_watch_get_enabled(watch)) {
    flags = dbus_watch_get_flags(watch);
    if (flags & DBUS_WATCH_READABLE) mask |= WL_EVENT_READABLE;
    if (flags & DBUS_WATCH_WRITABLE) mask |= WL_EVENT_WRITABLE;
  }

  wl_event_source_fd_update(event_source, mask);
}

static int
zn_dbus_connection_dispatch_timeout(void* data)
{
  DBusTimeout* timeout = data;

  if (dbus_timeout_get_enabled(timeout)) dbus_timeout_handle(timeout);

  return 0;
}

static int
zn_dbus_connection_adjust_timeout(
    DBusTimeout* timeout, struct wl_event_source* event_source)
{
  int64_t t = 0;

  if (dbus_timeout_get_enabled(timeout)) t = dbus_timeout_get_interval(timeout);

  return wl_event_source_timer_update(event_source, t);
}

static dbus_bool_t
zn_dbus_connection_add_timeout(DBusTimeout* timeout, void* data)
{
  struct wl_event_loop* loop = data;
  struct wl_event_source* event_source;
  int ret;

  event_source = wl_event_loop_add_timer(
      loop, zn_dbus_connection_dispatch_timeout, timeout);

  if (event_source == NULL) return FALSE;

  ret = zn_dbus_connection_adjust_timeout(timeout, event_source);
  if (ret < 0) {
    wl_event_source_remove(event_source);
    return FALSE;
  }

  dbus_timeout_set_data(timeout, event_source, NULL);

  return TRUE;
}

static void
zn_dbus_connection_remove_timeout(DBusTimeout* timeout, void* data)
{
  struct wl_event_source* event_source;
  (void)data;

  event_source = dbus_timeout_get_data(timeout);
  if (!event_source) return;

  wl_event_source_remove(event_source);
}

static void
zn_dbus_connection_toggle_timeout(DBusTimeout* timeout, void* data)
{
  struct wl_event_source* event_source;
  (void)data;

  event_source = dbus_timeout_get_data(timeout);
  if (!event_source) return;

  zn_dbus_connection_adjust_timeout(timeout, event_source);
}

ZN_EXPORT struct wl_event_source*
zn_dbus_connection_bind(DBusConnection* connection, struct wl_event_loop* loop)
{
  struct wl_event_source* event_source;
  bool ret;
  int fd;

  fd = eventfd(0, EFD_CLOEXEC);
  if (fd < 0) goto err;

  event_source = wl_event_loop_add_fd(
      loop, fd, 0, zn_dbus_connection_dispatch, connection);
  close(fd);

  if (event_source == NULL) goto err;

  wl_event_source_check(event_source);

  ret = dbus_connection_set_watch_functions(connection,
      zn_dbus_connection_add_watch, zn_dbus_connection_remove_watch,
      zn_dbus_connection_toggle_watch, loop, NULL);

  if (ret == false) goto err_source;

  ret = dbus_connection_set_timeout_functions(connection,
      zn_dbus_connection_add_timeout, zn_dbus_connection_remove_timeout,
      zn_dbus_connection_toggle_timeout, loop, NULL);

  if (ret == false) goto err_source;

  return event_source;

err_source:
  dbus_connection_set_timeout_functions(
      connection, NULL, NULL, NULL, NULL, NULL);
  dbus_connection_set_watch_functions(connection, NULL, NULL, NULL, NULL, NULL);
  wl_event_source_remove(event_source);

err:
  return NULL;
}

static void
zn_dbus_connection_unbind(
    DBusConnection* connection, struct wl_event_source* event_source)
{
  dbus_connection_set_timeout_functions(
      connection, NULL, NULL, NULL, NULL, NULL);
  dbus_connection_set_watch_functions(connection, NULL, NULL, NULL, NULL, NULL);

  if (event_source) wl_event_source_remove(event_source);
}

ZN_EXPORT DBusConnection*
zn_dbus_connection_create(DBusBusType type)
{
  DBusConnection* connection;
  DBusError err;

  dbus_connection_set_change_sigpipe(FALSE);

  dbus_error_init(&err);
  connection = dbus_bus_get_private(type, &err);
  if (connection == NULL) {
    if (dbus_error_is_set(&err)) {
      zn_log("dbus: failed to connect to dbus daemon: %s\n", err.message);
      dbus_error_free(&err);
    } else {
      zn_log("dbus: failed to connect to dbus daemon\n");
    }
    goto err;
  }

  dbus_connection_set_exit_on_disconnect(connection, FALSE);

  return connection;

err:
  return NULL;
}

ZN_EXPORT void
zn_dbus_connection_destroy(
    DBusConnection* connection, struct wl_event_source* event_source)
{
  zn_dbus_connection_unbind(connection, event_source);
  dbus_connection_close(connection);
  dbus_connection_unref(connection);
}
