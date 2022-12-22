#include "zen/data-device-manager.h"

#include <wlr/types/wlr_primary_selection_v1.h>
#include <zen-common.h>

struct zn_data_device_manager *
zn_data_device_manager_create(struct wl_display *display)
{
  struct zn_data_device_manager *self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->wlr_data_device_manager = wlr_data_device_manager_create(display);
  if (self->wlr_data_device_manager == NULL) {
    zn_error("Failed to create wlr_data_device_manager");
    goto err_free;
  }

  wlr_primary_selection_v1_device_manager_create(display);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_data_device_manager_destroy(struct zn_data_device_manager *self)
{
  free(self);
}
