#include "zen-desktop/xr-system.h"

#include <zen-desktop-protocol.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/xr-system.h"

static void zn_desktop_xr_system_destroy(struct zn_desktop_xr_system *self);

static void
zn_desktop_xr_system_handle_destroy(struct wl_resource *resource)
{
  wl_list_remove(wl_resource_get_link(resource));
}

static void
zn_desktop_xr_system_protocol_connect(
    struct wl_client *client UNUSED, struct wl_resource *resource UNUSED)
{}

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

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_desktop_xr_system_destroy(struct zn_desktop_xr_system *self)
{
  wl_list_remove(&self->zn_xr_system_destroy_listener.link);
  wl_global_destroy(self->global);
  wl_list_remove(&self->resource_list);
  free(self);
}
