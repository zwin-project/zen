#include "zen/scene/screen.h"

#include "zen-common.h"
#include "zen/scene/screen-layout.h"

struct zn_screen *
zn_screen_create(
    struct zn_screen_layout *screen_layout, struct zn_output *output)
{
  struct zn_screen *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->output = output;
  self->screen_layout = screen_layout;
  wl_list_init(&self->views);
  wl_signal_init(&self->events.destroy);

  zn_screen_layout_add(screen_layout, self);

  return self;

err:
  return NULL;
}

void
zn_screen_destroy(struct zn_screen *self)
{
  wl_signal_emit(&self->events.destroy, self);

  wl_list_remove(&self->views);
  zn_screen_layout_remove(self->screen_layout, self);
  free(self);
}
