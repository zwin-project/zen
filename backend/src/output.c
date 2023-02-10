#include "zen/backend/output.h"

#include <wayland-server-core.h>
#include <wayland-util.h>

#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"
#include "zen/screen.h"

static void zn_output_destroy(struct zn_output *self);

static void
zn_output_handle_wlr_output_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_output *self =
      zn_container_of(listener, self, wlr_output_destroy_listener);

  zn_output_destroy(self);
}

struct zn_output *
zn_output_create(struct wlr_output *wlr_output)
{
  struct zn_output *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->wlr_output = wlr_output;

  self->screen = zn_screen_create(self);
  if (self->screen == NULL) {
    zn_error("Failed to create a zn_screen");
    goto err_free;
  }

  self->wlr_output_destroy_listener.notify =
      zn_output_handle_wlr_output_destroy;
  wl_signal_add(
      &self->wlr_output->events.destroy, &self->wlr_output_destroy_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_output_destroy(struct zn_output *self)
{
  zn_screen_destroy(self->screen);
  wl_list_remove(&self->wlr_output_destroy_listener.link);
  free(self);
}
