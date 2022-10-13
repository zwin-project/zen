#include "zen/immersive/remote/display-system.h"

#include <wayland-server-core.h>
#include <znr/log.h>

#include "zen-common.h"

static void
zn_remote_immersive_display_system_handle_destroy(struct wl_resource* resource)
{
  wl_list_remove(&resource->link);
}

static void
zn_remote_immersive_display_system_send_applied(
    struct zn_remote_immersive_display_system* self)
{
  struct wl_resource* resource;

  wl_resource_for_each (resource, &self->resources) {
    zen_display_system_send_applied(resource, self->type);
  }
}

static void
zn_remote_immersive_display_system_protocol_switch_type(
    struct wl_client* client, struct wl_resource* resource, uint32_t type)
{
  struct zn_remote_immersive_display_system* self =
      wl_resource_get_user_data(resource);
  UNUSED(client);

  if (self->type == type) return;
  self->type = type;

  if (type == ZEN_DISPLAY_SYSTEM_TYPE_IMMERSIVE) {
    znr_remote_start(self->remote);
    wl_signal_emit(&self->base.events.activate, NULL);
    zn_remote_immersive_renderer_activate(self->renderer);
  } else {
    zn_remote_immersive_renderer_deactivate(self->renderer);
    wl_signal_emit(&self->base.events.deactivated, NULL);
    znr_remote_stop(self->remote);
  }

  zn_remote_immersive_display_system_send_applied(self);
}

static void
zn_remote_immersive_display_system_protocol_release(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static const struct zen_display_system_interface zn_display_system_interface = {
    .switch_type = zn_remote_immersive_display_system_protocol_switch_type,
    .release = zn_remote_immersive_display_system_protocol_release,
};

static void
zn_remote_immersive_display_system_bind(
    struct wl_client* client, void* data, uint32_t version, uint32_t id)
{
  struct zn_remote_immersive_display_system* self = data;
  struct wl_resource* resource;

  resource =
      wl_resource_create(client, &zen_display_system_interface, version, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    zn_error("Failed to create a wl_resource");
    return;
  }

  wl_resource_set_implementation(resource, &zn_display_system_interface, self,
      zn_remote_immersive_display_system_handle_destroy);

  wl_list_insert(&self->resources, &resource->link);

  zen_display_system_send_applied(resource, self->type);
}

struct zn_immersive_display_system*
zn_remote_immersive_display_system_create(struct wl_display* display,
    struct zn_remote_immersive_renderer* renderer, struct znr_remote* remote)
{
  struct zn_remote_immersive_display_system* self;

  znr_log_init();

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->global = wl_global_create(display, &zen_display_system_interface, 1,
      self, zn_remote_immersive_display_system_bind);

  self->type = ZEN_DISPLAY_SYSTEM_TYPE_SCREEN;
  wl_list_init(&self->resources);

  self->renderer = renderer;
  self->remote = remote;

  wl_signal_init(&self->base.events.activate);
  wl_signal_init(&self->base.events.deactivated);

  return &self->base;

err:
  return NULL;
}

void
zn_remote_immersive_display_system_destroy(
    struct zn_immersive_display_system* parent)
{
  struct zn_remote_immersive_display_system* self =
      zn_container_of(parent, self, base);

  wl_global_destroy(self->global);
  wl_list_remove(&self->resources);

  wl_list_remove(&self->base.events.activate.listener_list);
  wl_list_remove(&self->base.events.deactivated.listener_list);

  free(self);
}
