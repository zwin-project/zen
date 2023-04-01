#include "xr.h"

#include "xr-system-manager.h"
#include "zen-common/log.h"
#include "zen-common/util.h"

struct zn_xr *
zn_xr_create(struct wl_display *display)
{
  struct zn_xr *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->xr_system_manager = zn_xr_system_manager_create_remote(display);
  if (self->xr_system_manager == NULL) {
    zn_error("Failed to create zn_xr_system_manager");
    goto err_free;
  }

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_xr_destroy(struct zn_xr *self)
{
  zn_xr_system_manager_destroy(self->xr_system_manager);
  free(self);
}
