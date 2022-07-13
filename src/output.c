#include "output.h"

#include <zen-common.h>

struct zn_output {
  struct wlr_output *wlr_output;

  struct wl_listener wlr_output_destroy_listener;
};

static void zn_output_destroy(struct zn_output *self);

static void
zn_output_wlr_output_destroy_handler(struct wl_listener *listener, void *data)
{
  struct zn_output *output =
      zn_container_of(listener, output, wlr_output_destroy_listener);
  UNUSED(data);

  zn_output_destroy(output);
}

struct zn_output *
zn_output_create(struct wlr_output *wlr_output)
{
  struct zn_output *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wlr_output->data = self;
  self->wlr_output = wlr_output;

  self->wlr_output_destroy_listener.notify =
      zn_output_wlr_output_destroy_handler;
  wl_signal_add(
      &self->wlr_output->events.destroy, &self->wlr_output_destroy_listener);

  return self;

err:
  return NULL;
}

static void
zn_output_destroy(struct zn_output *self)
{
  free(self);
}
