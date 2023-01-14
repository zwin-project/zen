#include "backend.h"

#include <zen-common.h>

#include "compositor.h"
#include "zwnr/shm.h"

int
zwnr_backend_activate(struct zwnr_backend *parent)
{
  struct zwnr_backend_impl *self = zn_container_of(parent, self, base);
  return zwnr_compositor_activate(self->compositor);
}

void
zwnr_backend_deactivate(struct zwnr_backend *parent)
{
  struct zwnr_backend_impl *self = zn_container_of(parent, self, base);
  zwnr_compositor_deactivate(self->compositor);
}

struct zwnr_backend *
zwnr_backend_create(struct wl_display *display)
{
  struct zwnr_backend_impl *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->display = display;

  zwnr_shm_init(display);

  self->compositor = zwnr_compositor_create(self->display, self);
  if (self->compositor == NULL) {
    zn_error("Failed to create a zwnr_compositor");
    goto err_free;
  }

  wl_signal_init(&self->base.events.new_virtual_object);

  return &self->base;

err_free:
  free(self);

err:
  return NULL;
}

void
zwnr_backend_destroy(struct zwnr_backend *parent)
{
  struct zwnr_backend_impl *self = zn_container_of(parent, self, base);

  zwnr_compositor_destroy(self->compositor);

  wl_list_remove(&self->base.events.new_virtual_object.listener_list);

  free(self);
}
