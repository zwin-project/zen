#include "backend.h"

#include <zen-common.h>

#include "compositor.h"
#include "seat.h"
#include "zgnr/shm.h"

int
zgnr_backend_activate(struct zgnr_backend *parent)
{
  struct zgnr_backend_impl *self = zn_container_of(parent, self, base);
  return zgnr_compositor_activate(self->compositor);
}

void
zgnr_backend_deactivate(struct zgnr_backend *parent)
{
  struct zgnr_backend_impl *self = zn_container_of(parent, self, base);
  zgnr_compositor_deactivate(self->compositor);
}

struct zgnr_backend *
zgnr_backend_create(struct wl_display *display)
{
  struct zgnr_backend_impl *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->display = display;

  zgnr_shm_init(display);

  self->compositor = zgnr_compositor_create(self->display, self);
  if (self->compositor == NULL) {
    zn_error("Failed to create a zgnr_compositor");
    goto err_free;
  }

  self->seat = zgnr_seat_create(self->display);
  if (self->seat == NULL) {
    zn_error("Failed to creat a zgnr_seat");
    goto err_compositor;
  }

  wl_signal_init(&self->base.events.new_virtual_object);

  return &self->base;

err_compositor:
  zgnr_compositor_destroy(self->compositor);

err_free:
  free(self);

err:
  return NULL;
}

void
zgnr_backend_destroy(struct zgnr_backend *parent)
{
  struct zgnr_backend_impl *self = zn_container_of(parent, self, base);

  zgnr_seat_destroy(self->seat);
  zgnr_compositor_destroy(self->compositor);

  wl_list_remove(&self->base.events.new_virtual_object.listener_list);

  free(self);
}
