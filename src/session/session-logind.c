#include <dbus/dbus.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysmacros.h>
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
  char* session_path;

  DBusConnection* dbus;
  struct wl_event_source* dbus_event_source;
  DBusPendingCall* pending_active;
};

static void
zn_session_logind_active_prop_changed(
    struct zn_session_logind* self, DBusMessage* message, DBusMessageIter* iter)
{
  DBusMessageIter sub;
  dbus_bool_t active;
  UNUSED(message);

  if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_VARIANT) return;

  dbus_message_iter_recurse(iter, &sub);

  if (dbus_message_iter_get_arg_type(&sub) != DBUS_TYPE_BOOLEAN) return;

  dbus_message_iter_get_basic(&sub, &active);

  wl_signal_emit(&self->base.session_signal, &active);
}

static void
zn_session_logind_get_active_callback(DBusPendingCall* pending, void* data)
{
  struct zn_session_logind* self = data;
  DBusMessageIter iter;
  DBusMessage* message;
  int type;

  dbus_pending_call_unref(self->pending_active);
  self->pending_active = NULL;

  message = dbus_pending_call_steal_reply(pending);
  if (message == NULL) return;

  type = dbus_message_get_type(message);
  if (type == DBUS_MESSAGE_TYPE_METHOD_RETURN &&
      dbus_message_iter_init(message, &iter))
    zn_session_logind_active_prop_changed(self, message, &iter);

  dbus_message_unref(message);
}

static void
zn_session_logind_get_active(struct zn_session_logind* self)
{
  DBusPendingCall* pending;
  DBusMessage* message;
  bool ret;
  const char *interface, *name;

  message = dbus_message_new_method_call("org.freedesktop.login1",
      self->session_path, "org.freedesktop.DBus.Properties", "Get");
  if (message == NULL) return;

  interface = "org.freedesktop.login1.Session";
  name = "Active";

  ret = dbus_message_append_args(message, DBUS_TYPE_STRING, &interface,
      DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID);
  if (ret == false) goto err_unref;

  ret = dbus_connection_send_with_reply(self->dbus, message, &pending, -1);
  if (ret == false) goto err_unref;

  ret = dbus_pending_call_set_notify(
      pending, zn_session_logind_get_active_callback, self, NULL);
  if (ret == false) {
    dbus_pending_call_cancel(pending);
    dbus_pending_call_unref(pending);
    goto err_unref;
  }

  if (self->pending_active) {
    dbus_pending_call_cancel(self->pending_active);
    dbus_pending_call_unref(self->pending_active);
  }

  self->pending_active = pending;

err_unref:
  dbus_message_unref(message);
}

static void
zn_session_logind_pause_device_complete(
    struct zn_session_logind* self, uint32_t major, uint32_t minor)
{
  DBusMessage* message;
  bool ret;

  message =
      dbus_message_new_method_call("org.freedesktop.login1", self->session_path,
          "org.freedesktop.login1.Session", "PauseDeviceComplete");
  if (message == NULL) return;

  ret = dbus_message_append_args(message, DBUS_TYPE_UINT32, &major,
      DBUS_TYPE_UINT32, &minor, DBUS_TYPE_INVALID);

  if (ret) dbus_connection_send(self->dbus, message, NULL);
  dbus_message_unref(message);
}

static void
zn_session_logind_disconnected_dbus(struct zn_session_logind* self)
{
  UNUSED(self);
  zn_log("logind: dbus connection lost, exiting..\n");
  exit(-1);
}

static void
zn_session_logind_session_removed(
    struct zn_session_logind* self, DBusMessage* message)
{
  const char *name, *obj;
  bool ret;

  ret = dbus_message_get_args(message, NULL, DBUS_TYPE_STRING, &name,
      DBUS_TYPE_STRING, &obj, DBUS_TYPE_INVALID);
  if (ret == false) {
    zn_log("logind: cannot parse SessionRemoved dbus signal\n");
    return;
  }

  if (strcmp(name, self->session_id) == 0) {
    zn_log("logind: out session got lost, exiting..\n");
    exit(-1);
  }
}

static void
zn_session_logind_property_changed(
    struct zn_session_logind* self, DBusMessage* message)
{
  DBusMessageIter iter, sub, entry;
  const char *interface, *name;

  // check interface_name
  if (!dbus_message_iter_init(message, &iter) ||
      dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_STRING)
    goto error;

  dbus_message_iter_get_basic(&iter, &interface);

  // check changed_properties
  if (!dbus_message_iter_next(&iter) ||
      dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
    goto error;

  dbus_message_iter_recurse(&iter, &sub);

  while (dbus_message_iter_get_arg_type(&sub) == DBUS_TYPE_DICT_ENTRY) {
    dbus_message_iter_recurse(&sub, &entry);

    if (dbus_message_iter_get_arg_type(&entry) != DBUS_TYPE_STRING) goto error;

    dbus_message_iter_get_basic(&entry, &name);
    if (!dbus_message_iter_next(&entry)) goto error;

    if (!strcmp(name, "Active")) {
      zn_session_logind_active_prop_changed(self, message, &entry);
      return;
    }

    dbus_message_iter_next(&sub);
  }

  // check invalidated_properties
  if (!dbus_message_iter_next(&iter) ||
      dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
    goto error;

  dbus_message_iter_recurse(&iter, &sub);

  while (dbus_message_iter_get_arg_type(&sub) == DBUS_TYPE_STRING) {
    dbus_message_iter_get_basic(&sub, &name);

    if (!strcmp(name, "Active")) {
      zn_session_logind_get_active(self);
      return;
    }

    dbus_message_iter_next(&sub);
  }

  return;

error:
  zn_log("logind: cannot parse PropertiesChanged dbus signal\n");
}

static void
zn_session_logind_device_paused(
    struct zn_session_logind* self, DBusMessage* message)
{
  struct zn_session_device_changed_signal_arg arg;
  bool ret;
  const char* type;
  uint32_t major, minor;

  ret = dbus_message_get_args(message, NULL, DBUS_TYPE_UINT32, &major,
      DBUS_TYPE_UINT32, &minor, DBUS_TYPE_STRING, &type, DBUS_TYPE_INVALID);

  if (ret == false) {
    zn_log("logind: cannot parse PauseDevice dbus signal\n");
    return;
  }

  if (strcmp(type, "pause") == 0)
    zn_session_logind_pause_device_complete(self, major, minor);

  arg.added = false;
  arg.device = makedev(major, minor);
  wl_signal_emit(&self->base.device_changed_signal, &arg);
}

static void
zn_session_logind_device_resumed(
    struct zn_session_logind* self, DBusMessage* message)
{
  struct zn_session_device_changed_signal_arg arg;
  bool ret;
  uint32_t major, minor;

  ret = dbus_message_get_args(message, NULL, DBUS_TYPE_UINT32, &major,
      DBUS_TYPE_UINT32, &minor, DBUS_TYPE_INVALID);

  if (ret == false) {
    zn_log("logind: cannot parse ResumeDevice dbus signal\n");
    return;
  }

  arg.added = true;
  arg.device = makedev(major, minor);
  wl_signal_emit(&self->base.device_changed_signal, &arg);
}

static DBusHandlerResult
zn_session_logind_filter_dbus(
    DBusConnection* connection, DBusMessage* message, void* data)
{
  struct zn_session_logind* self = data;
  UNUSED(connection);

  if (dbus_message_is_signal(message, DBUS_INTERFACE_LOCAL, "Disconnected")) {
    zn_session_logind_disconnected_dbus(self);
  } else if (dbus_message_is_signal(
                 message, "org.freedesktop.login1.Manager", "SessionRemoved")) {
    zn_session_logind_session_removed(self, message);
  } else if (dbus_message_is_signal(message, "org.freedesktop.DBus.Properties",
                 "PropertiesChanged")) {
    zn_session_logind_property_changed(self, message);
  } else if (dbus_message_is_signal(
                 message, "org.freedesktop.login1.Session", "PauseDevice")) {
    zn_session_logind_device_paused(self, message);
  } else if (dbus_message_is_signal(
                 message, "org.freedesktop.login1.Session", "ResumeDevice")) {
    zn_session_logind_device_resumed(self, message);
  }

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static int
zn_session_logind_setup_dbus(struct zn_session_logind* self)
{
  bool ret_bool;
  int ret;

  ret = asprintf(&self->session_path, "/org/freedesktop/login1/session/%s",
      self->session_id);
  if (ret < 0) goto err;

  ret_bool = dbus_connection_add_filter(
      self->dbus, zn_session_logind_filter_dbus, self, NULL);
  if (ret_bool == false) {
    zn_log("logind: failed to add dbus filter\n");
    goto err_session_path;
  }

  ret = zn_dbus_connection_add_match_signal(self->dbus,
      "org.freedesktop.login1", "org.freedesktop.login1.Manager",
      "SessionRemoved", "/org/freedesktop/login1");
  if (ret < 0) {
    zn_log("logind: failed to add dbus match\n");
    goto err_session_path;
  }

  ret =
      zn_dbus_connection_add_match_signal(self->dbus, "org.freedesktop.login1",
          "org.freedesktop.login1.Session", "PauseDevice", self->session_path);
  if (ret < 0) {
    zn_log("logind: failed to add dbus match\n");
    goto err_session_path;
  }

  ret =
      zn_dbus_connection_add_match_signal(self->dbus, "org.freedesktop.login1",
          "org.freedesktop.login1.Session", "ResumeDevice", self->session_path);
  if (ret < 0) {
    zn_log("logind: failed to add dbus match\n");
    goto err_session_path;
  }

  ret = zn_dbus_connection_add_match_signal(self->dbus,
      "org.freedesktop.login1", "org.freedesktop.DBus.Properties",
      "PropertiesChanged", self->session_path);
  if (ret < 0) {
    zn_log("logind: failed to add dbus match\n");
    goto err_session_path;
  }

  return 0;

err_session_path:
  /* don't need to remove match signals, the connection will be closed anyway */
  free(self->session_path);
  self->session_path = NULL;

err:
  return -1;
}

static void
zn_session_logind_teardown_dbus(struct zn_session_logind* self)
{
  /* don't need to remove match signals, the connection will be closed anyway */
  free(self->session_path);
}

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

static int
zn_session_logind_take_control(struct zn_session_logind* self)
{
  DBusError err;
  DBusMessage *message, *reply;
  dbus_bool_t force;
  bool ret;

  dbus_error_init(&err);

  message = dbus_message_new_method_call("org.freedesktop.login1",
      self->session_path, "org.freedesktop.login1.Session", "TakeControl");
  if (message == NULL) goto err;

  force = false;
  ret = dbus_message_append_args(
      message, DBUS_TYPE_BOOLEAN, &force, DBUS_TYPE_INVALID);
  if (ret == false) goto err_unref;

  reply =
      dbus_connection_send_with_reply_and_block(self->dbus, message, -1, &err);
  if (reply == NULL) {
    if (dbus_error_has_name(&err, DBUS_ERROR_UNKNOWN_METHOD))
      zn_log("logind: old systemd version detected\n");
    else
      zn_log("logind: cannot take control over session %s\n", self->session_id);

    if (dbus_error_is_set(&err)) dbus_error_free(&err);

    goto err_unref;
  }

  dbus_message_unref(reply);
  dbus_message_unref(message);

  return 0;

err_unref:
  dbus_message_unref(message);

err:
  return -1;
}

static void
zn_session_logind_release_control(struct zn_session_logind* self)
{
  DBusMessage* message;

  message = dbus_message_new_method_call("org.freedesktop.login1",
      self->session_path, "org.freedesktop.login1.Session", "ReleaseControl");
  if (message == NULL) return;

  dbus_connection_send(self->dbus, message, NULL);
  dbus_message_unref(message);
}

static int
zn_session_logind_activate(struct zn_session_logind* self)
{
  DBusMessage* message;

  message = dbus_message_new_method_call("org.freedesktop.login1",
      self->session_path, "org.freedesktop.login1.Session", "Activate");

  if (message == NULL) return -1;

  dbus_connection_send(self->dbus, message, NULL);
  return 0;
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

  ret = zn_session_logind_setup_dbus(self);
  if (ret < 0) goto err_dbus;

  ret = zn_session_logind_take_control(self);
  if (ret < 0) goto err_dbus_cleanup;

  ret = zn_session_logind_activate(self);
  if (ret < 0) goto err_dbus_cleanup;

  zn_log("logind: session control granted\n");

  return 0;

err_dbus_cleanup:
  zn_session_logind_teardown_dbus(self);

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
  wl_signal_init(&self->base.device_changed_signal);
  wl_signal_init(&self->base.session_signal);

  return &self->base;

err:
  return NULL;
}

ZN_EXPORT void
zn_session_destroy(struct zn_session* parent)
{
  struct zn_session_logind* self = zn_container_of(parent, self, base);

  if (self->pending_active) {
    dbus_pending_call_cancel(self->pending_active);
    dbus_pending_call_unref(self->pending_active);
  }

  zn_session_logind_release_control(self);
  zn_dbus_connection_destroy(self->dbus, self->dbus_event_source);
  zn_session_logind_teardown_dbus(self);
  free(self->session_id);
  free(self);
}
