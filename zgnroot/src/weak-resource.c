#include "weak-resource.h"

#include <zen-common.h>

static void
zgnr_weak_resource_handle_destroy(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zgnr_weak_resource *self =
      zn_container_of(listener, self, destroy_listener);

  zgnr_weak_resource_unlink(self);
}

void
zgnr_weak_resource_init(struct zgnr_weak_resource *self)
{
  self->resource = NULL;
  self->destroy_listener.notify = zgnr_weak_resource_handle_destroy;
  wl_list_init(&self->destroy_listener.link);
}

void *
zgnr_weak_resource_get_user_data(struct zgnr_weak_resource *self)
{
  if (self->resource) return wl_resource_get_user_data(self->resource);
  return NULL;
}

void
zgnr_weak_resource_link(
    struct zgnr_weak_resource *self, struct wl_resource *resource)
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
zgnr_weak_resource_unlink(struct zgnr_weak_resource *self)
{
  zgnr_weak_resource_link(self, NULL);
}
