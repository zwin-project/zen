#include "input-manager.h"

#include <wayland-server.h>

#include "zen-common.h"

struct zn_input_manager {
  struct wl_list devices;  // zn_input_device::link
};

struct zn_input_manager*
zn_input_manager_create(void)
{
  struct zn_input_manager* self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_list_init(&self->devices);

  return self;

err:
  return NULL;
}

void
zn_input_manager_destroy(struct zn_input_manager* self)
{
  wl_list_remove(&self->devices);
  free(self);
}
