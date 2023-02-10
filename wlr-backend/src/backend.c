#include "zen/wlr-backend/backend.h"

#include <wayland-server-core.h>
#include <wayland-util.h>

#include "zen-common/log.h"
#include "zen-common/util.h"

static struct zn_wlr_backend *
zn_wlr_backend_get(struct zn_backend *base)
{
  struct zn_wlr_backend *self = zn_container_of(base, self, base);
  return self;
}

struct zn_backend *
zn_backend_create(struct wl_display *display)
{
  struct zn_wlr_backend *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->display = display;
  wl_signal_init(&self->base.events.view_mapped);

  return &self->base;

err:
  return NULL;
}

void
zn_backend_destroy(struct zn_backend *base)
{
  struct zn_wlr_backend *self = zn_wlr_backend_get(base);

  wl_list_remove(&self->base.events.view_mapped.listener_list);
  free(self);
}
