#include "zen-common/weak-resource.h"

#include <zen-common.h>

#include "zen-common/util.h"

static void
zn_weak_resource_handle_destroy(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_weak_resource *self =
      zn_container_of(listener, self, destroy_listener);

  zn_weak_resource_unlink(self);
}

void
zn_weak_resource_init(struct zn_weak_resource *self)
{
  self->resource = NULL;
  self->destroy_listener.notify = zn_weak_resource_handle_destroy;
  wl_list_init(&self->destroy_listener.link);
}

void *
zn_weak_resource_get_user_data(struct zn_weak_resource *self)
{
  if (self->resource) return wl_resource_get_user_data(self->resource);
  return NULL;
}

void
zn_weak_resource_link(
    struct zn_weak_resource *self, struct wl_resource *resource)
{
  if (self->resource) {
    wl_list_remove(&self->destroy_listener.link);
    wl_list_init(&self->destroy_listener.link);
  }

  if (resource) {
    wl_resource_add_destroy_listener(resource, &self->destroy_listener);
  }

  self->resource = resource;
}

void
zn_weak_resource_unlink(struct zn_weak_resource *self)
{
  zn_weak_resource_link(self, NULL);
}
