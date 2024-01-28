#include "xr.h"

#include "xr-compositor.h"
#include "xr-shell.h"
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

  self->xr_compositor = zn_xr_compositor_create(display);
  if (self->xr_compositor == NULL) {
    zn_error("Failed to create zn_xr_compositor");
    goto err_free_system_manager;
  }

  self->xr_shell = zn_xr_shell_create(display);
  if (self->xr_shell == NULL) {
    zn_error("Failed to create zn_xr_shell");
    goto err_xr_compositor;
  }

  return self;

err_xr_compositor:
  zn_xr_compositor_destroy(self->xr_compositor);

err_free_system_manager:
  zn_xr_system_manager_destroy(self->xr_system_manager);

err_free:
  free(self);

err:
  return NULL;
}

void
zn_xr_destroy(struct zn_xr *self)
{
  zn_xr_shell_destroy(self->xr_shell);
  zn_xr_compositor_destroy(self->xr_compositor);
  zn_xr_system_manager_destroy(self->xr_system_manager);
  free(self);
}
