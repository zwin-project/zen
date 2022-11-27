#include "bounded.h"

#include <cglm/vec3.h>
#include <zen-common.h>
#include <zigen-shell-protocol.h>

#include "bounded-configure.h"
#include "region.h"

static void zgnr_bounded_destroy(struct zgnr_bounded_impl *self);
static void zgnr_bounded_inert(struct zgnr_bounded_impl *self);

void
zgnr_bounded_configure(struct zgnr_bounded_impl *self, vec3 half_size)
{
  struct zgnr_virtual_object_impl *virtual_object =
      zn_container_of(self->base.virtual_object, virtual_object, base);

  struct zgnr_bounded_configure *configure =
      zgnr_bounded_configure_create(virtual_object->display, half_size);

  if (configure == NULL) {
    zn_error("Failed to create bounded configure");
    return;
  }

  {
    struct wl_array array;
    zn_vec3_to_array(configure->half_size, &array);
    zgn_bounded_send_configure(self->resource, &array, configure->serial);
    wl_array_release(&array);
  }

  wl_list_insert(self->configure_list.prev, &configure->link);
}

static void
zgnr_bounded_handle_destroy(struct wl_resource *resource)
{
  struct zgnr_bounded_impl *self = wl_resource_get_user_data(resource);

  zgnr_bounded_destroy(self);
}

static void
zgnr_bounded_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zgnr_bounded_protocol_ack_configure(
    struct wl_client *client, struct wl_resource *resource, uint32_t serial)
{
  UNUSED(client);
  struct zgnr_bounded_impl *self = wl_resource_get_user_data(resource);
  if (self == NULL) return;

  bool found = false;
  struct zgnr_bounded_configure *configure, *tmp;
  wl_list_for_each (configure, &self->configure_list, link) {
    if (configure->serial == serial) {
      found = true;
      break;
    }
  }

  if (!found) {
    wl_resource_post_error(self->resource, ZGN_SHELL_ERROR_INVALID_STATE,
        "wrong configure serial: %u", serial);
    return;
  }

  wl_list_for_each_safe (configure, tmp, &self->configure_list, link) {
    if (configure->serial == serial) {
      break;
    }
    zgnr_bounded_configure_destroy(configure);
  }

  self->configured = true;
  glm_vec3_copy(configure->half_size, self->pending.half_size);
  self->pending.damage |= ZGNR_BOUNDED_DAMAGE_HALF_SIZE;

  zgnr_bounded_configure_destroy(configure);
}

static void
zgnr_bounded_protocol_set_region(struct wl_client *client,
    struct wl_resource *resource, struct wl_resource *region_resource)
{
  UNUSED(client);
  struct zgnr_bounded_impl *self = wl_resource_get_user_data(resource);
  if (self == NULL) return;

  if (self->pending.region) {
    zgnr_region_node_destroy(self->pending.region);
  }

  if (region_resource == NULL) {
    self->pending.region = NULL;
  } else {
    struct zgnr_region *region = wl_resource_get_user_data(region_resource);
    self->pending.region = zgnr_region_node_create_copy(region->node);
  }

  self->pending.damage |= ZGNR_BOUNDED_DAMAGE_REGION;
}

static void
zgnr_bounded_protocol_move(struct wl_client *client,
    struct wl_resource *resource, struct wl_resource *seat_resource,
    uint32_t serial)
{
  struct zgnr_bounded_impl *self = wl_resource_get_user_data(resource);
  struct zgnr_seat *seat = wl_resource_get_user_data(seat_resource);
  struct zgnr_bounded_move_event event;

  event.serial = serial;
  event.seat = seat;
  event.client = client;

  wl_signal_emit(&self->base.events.move, &event);
}

static const struct zgn_bounded_interface implementation = {
    .destroy = zgnr_bounded_protocol_destroy,
    .ack_configure = zgnr_bounded_protocol_ack_configure,
    .set_region = zgnr_bounded_protocol_set_region,
    .move = zgnr_bounded_protocol_move,
};

static void
zgnr_bounded_handle_virtual_object_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zgnr_bounded_impl *self =
      zn_container_of(listener, self, virtual_object_destroy_listener);

  zgnr_bounded_inert(self);
}

static void
zgnr_bounded_handle_virtual_object_commit(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zgnr_bounded_impl *self =
      zn_container_of(listener, self, virtual_object_commit_listener);

  if (!self->configured) {
    wl_resource_post_error(self->resource, ZGN_BOUNDED_ERROR_UNCONFIGURED,
        "zgn_bounded was committed before configured");
    return;
  }

  if (self->pending.damage & ZGNR_BOUNDED_DAMAGE_HALF_SIZE) {
    glm_vec3_copy(self->pending.half_size, self->base.current.half_size);
  }

  if (self->pending.damage & ZGNR_BOUNDED_DAMAGE_REGION) {
    if (self->base.current.region) {
      zgnr_region_node_destroy(self->base.current.region);
    }
    self->base.current.region = self->pending.region;  // move ownership
    self->pending.region = NULL;
  }

  self->pending.damage = 0;
}

static void
zgnr_bounded_inert(struct zgnr_bounded_impl *self)
{
  struct wl_resource *resource = self->resource;
  zgnr_bounded_destroy(self);
  wl_resource_set_user_data(resource, NULL);
  wl_resource_set_destructor(resource, NULL);
}

struct zgnr_bounded_impl *
zgnr_bounded_create(struct wl_client *client, uint32_t id,
    struct zgnr_virtual_object_impl *virtual_object)
{
  struct zgnr_bounded_impl *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->base.virtual_object = &virtual_object->base;

  wl_signal_init(&self->base.events.destroy);
  wl_signal_init(&self->base.events.move);
  wl_list_init(&self->configure_list);

  self->configured = false;

  self->pending.region = NULL;
  self->pending.damage = 0;
  self->base.current.region = NULL;

  self->virtual_object_destroy_listener.notify =
      zgnr_bounded_handle_virtual_object_destroy;
  wl_signal_add(&virtual_object->base.events.destroy,
      &self->virtual_object_destroy_listener);

  self->virtual_object_commit_listener.notify =
      zgnr_bounded_handle_virtual_object_commit;
  wl_signal_add(
      &virtual_object->events.on_commit, &self->virtual_object_commit_listener);

  self->resource = wl_resource_create(client, &zgn_bounded_interface, 1, id);
  if (self->resource == NULL) {
    zn_error("Failed to create a wl_resource");
    goto err_free;
  }

  wl_resource_set_implementation(
      self->resource, &implementation, self, zgnr_bounded_handle_destroy);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zgnr_bounded_destroy(struct zgnr_bounded_impl *self)
{
  wl_signal_emit(&self->base.events.destroy, NULL);

  struct zgnr_bounded_configure *configure, *tmp;
  wl_list_for_each_safe (configure, tmp, &self->configure_list, link) {
    zgnr_bounded_configure_destroy(configure);
  }

  if (self->pending.region) {
    zgnr_region_node_destroy(self->pending.region);
  }

  if (self->base.current.region) {
    zgnr_region_node_destroy(self->base.current.region);
  }

  wl_list_remove(&self->virtual_object_destroy_listener.link);
  wl_list_remove(&self->virtual_object_commit_listener.link);
  wl_list_remove(&self->base.events.destroy.listener_list);
  wl_list_remove(&self->base.events.move.listener_list);

  free(self);
}
