#include "zen-desktop/xr-system.h"

#include <zen-desktop-protocol.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/backend.h"
#include "zen/server.h"
#include "zen/xr-system.h"

static void zn_desktop_xr_system_destroy(struct zn_desktop_xr_system *self);

static void
zn_desktop_xr_system_handle_destroy(struct wl_resource *resource)
{
  wl_list_remove(wl_resource_get_link(resource));
}

static void
zn_desktop_xr_system_protocol_connect(
    struct wl_client *client UNUSED, struct wl_resource *resource)
{
  struct zn_desktop_xr_system *self = wl_resource_get_user_data(resource);

  zn_xr_system_connect(self->zn_xr_system);
}

static const struct zen_xr_system_interface implementation = {
    .connect = zn_desktop_xr_system_protocol_connect,
};

static void
zn_desktop_xr_system_bind(
    struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
  struct zn_desktop_xr_system *self = data;

  struct wl_resource *resource =
      wl_resource_create(client, &zen_xr_system_interface, (int)version, id);
  if (resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(
      resource, &implementation, self, zn_desktop_xr_system_handle_destroy);

  wl_list_insert(&self->resource_list, wl_resource_get_link(resource));
}

static void
zn_desktop_xr_system_handle_xr_system_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_desktop_xr_system *self =
      zn_container_of(listener, self, zn_xr_system_destroy_listener);

  zn_desktop_xr_system_destroy(self);
}

static void
zn_desktop_xr_system_handle_session_state_changed(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_server *server = zn_server_get_singleton();

  struct zn_desktop_xr_system *self =
      zn_container_of(listener, self, session_state_changed_listener);

  struct wl_resource *resource = NULL;

  enum zen_xr_system_status status = self->status;

  switch (self->zn_xr_system->state) {
    case ZN_XR_SYSTEM_SESSION_STATE_AVAILABLE:
      status = ZEN_XR_SYSTEM_STATUS_AVAILABLE;
      break;
    case ZN_XR_SYSTEM_SESSION_STATE_SYNCHRONIZED:
      if (!zn_backend_get_xr_system(server->backend)) {
        zn_backend_set_xr_system(server->backend, self->zn_xr_system);
      }
      status = ZEN_XR_SYSTEM_STATUS_CONNECTED;
      break;
    case ZN_XR_SYSTEM_SESSION_STATE_DEAD:
      status = ZEN_XR_SYSTEM_STATUS_UNAVAILABLE;
      break;
    case ZN_XR_SYSTEM_SESSION_STATE_VISIBLE:  // fallthrough
    case ZN_XR_SYSTEM_SESSION_STATE_FOCUS:
      return;
  }

  if (self->status != status) {
    self->status = status;
    wl_resource_for_each (resource, &self->resource_list) {
      zen_xr_system_send_status(resource, status);
    }
  }
}

struct zn_desktop_xr_system *
zn_desktop_xr_system_create(
    struct zn_xr_system *zn_xr_system, struct wl_display *display)
{
  struct zn_desktop_xr_system *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zn_xr_system = zn_xr_system;
  self->status = ZEN_XR_SYSTEM_STATUS_AVAILABLE;
  wl_list_init(&self->resource_list);

  self->global = wl_global_create(
      display, &zen_xr_system_interface, 1, self, zn_desktop_xr_system_bind);
  if (self->global == NULL) {
    zn_error("Failed to create a wl_global");
    goto err_free;
  }

  self->zn_xr_system_destroy_listener.notify =
      zn_desktop_xr_system_handle_xr_system_destroy;
  wl_signal_add(
      &zn_xr_system->events.destroy, &self->zn_xr_system_destroy_listener);

  self->session_state_changed_listener.notify =
      zn_desktop_xr_system_handle_session_state_changed;
  wl_signal_add(&zn_xr_system->events.session_state_changed,
      &self->session_state_changed_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_desktop_xr_system_destroy(struct zn_desktop_xr_system *self)
{
  wl_list_remove(&self->session_state_changed_listener.link);
  wl_list_remove(&self->zn_xr_system_destroy_listener.link);
  wl_global_destroy(self->global);
  wl_list_remove(&self->resource_list);
  free(self);
}
