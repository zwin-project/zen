#include "expansive.h"

#include <zen-common.h>
#include <zwin-shell-protocol.h>

#include "region.h"

static void zwnr_expansive_destroy(struct zwnr_expansive_impl *self);
static void zwnr_expansive_inert(struct zwnr_expansive_impl *self);

static void
zwn_expansive_handle_destroy(struct wl_resource *resource)
{
  struct zwnr_expansive_impl *self = wl_resource_get_user_data(resource);

  zwnr_expansive_destroy(self);
}

static void
zwn_expansive_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);

  wl_resource_destroy(resource);
}

static void
zwn_expansive_protocol_set_region(struct wl_client *client,
    struct wl_resource *resource, struct wl_resource *region_resource)
{
  UNUSED(client);
  struct zwnr_expansive_impl *self = wl_resource_get_user_data(resource);
  if (self == NULL) return;

  if (self->pending.region) {
    zwnr_region_node_destroy(self->pending.region);
  }

  if (region_resource == NULL) {
    self->pending.region = NULL;
  } else {
    struct zwnr_region *region = wl_resource_get_user_data(region_resource);
    self->pending.region = zwnr_region_node_create_copy(region->node);
  }

  self->pending.damage |= ZWNR_EXPANSIVE_DAMAGE_REGION;
}

static const struct zwn_expansive_interface implementation = {
    .destroy = zwn_expansive_protocol_destroy,
    .set_region = zwn_expansive_protocol_set_region,
};

void
zwnr_expansive_handle_virtual_object_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zwnr_expansive_impl *self =
      zn_container_of(listener, self, virtual_object_destroy_listener);

  zwnr_expansive_inert(self);
}

static void
zwnr_expansive_handle_virtual_object_commit(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zwnr_expansive_impl *self =
      zn_container_of(listener, self, virtual_object_commit_listener);

  if (self->pending.damage & ZWNR_EXPANSIVE_DAMAGE_REGION) {
    if (self->base.current.region) {
      zwnr_region_node_destroy(self->base.current.region);
    }
    self->base.current.region = self->pending.region;  // move ownership
    self->pending.region = NULL;
  }

  self->pending.damage = 0;
}

static void
zwnr_expansive_inert(struct zwnr_expansive_impl *self)
{
  struct wl_resource *resource = self->resource;
  zwnr_expansive_destroy(self);
  wl_resource_set_user_data(resource, NULL);
  wl_resource_set_destructor(resource, NULL);
}

struct zwnr_expansive_impl *
zwnr_expansive_create(struct wl_client *client, uint32_t id,
    struct zwnr_virtual_object_impl *virtual_object)
{
  struct zwnr_expansive_impl *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory ");
    goto err;
  }

  self->base.virtual_object = &virtual_object->base;
  self->pending.region = NULL;
  self->pending.damage = 0;
  self->base.current.region = NULL;

  self->resource = wl_resource_create(client, &zwn_expansive_interface, 1, id);
  if (self->resource == NULL) {
    zn_error("Failed to create a wl_resoure");
    goto err_free;
  }

  wl_resource_set_implementation(
      self->resource, &implementation, self, zwn_expansive_handle_destroy);

  wl_signal_init(&self->base.events.destroy);

  self->virtual_object_destroy_listener.notify =
      zwnr_expansive_handle_virtual_object_destroy;
  wl_signal_add(&virtual_object->base.events.destroy,
      &self->virtual_object_destroy_listener);

  self->virtual_object_commit_listener.notify =
      zwnr_expansive_handle_virtual_object_commit;
  wl_signal_add(
      &virtual_object->events.on_commit, &self->virtual_object_commit_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zwnr_expansive_destroy(struct zwnr_expansive_impl *self)
{
  wl_signal_emit(&self->base.events.destroy, NULL);

  if (self->pending.region) {
    zwnr_region_node_destroy(self->pending.region);
  }

  if (self->base.current.region) {
    zwnr_region_node_destroy(self->base.current.region);
  }

  wl_list_remove(&self->base.events.destroy.listener_list);
  wl_list_remove(&self->virtual_object_destroy_listener.link);
  wl_list_remove(&self->virtual_object_commit_listener.link);
  free(self);
}
