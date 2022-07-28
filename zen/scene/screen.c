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
  zn_screen_layout_add(screen_layout, self);
  wl_list_init(&self->views);

  return self;

err:
  return NULL;
}

void
zn_screen_destroy(struct zn_screen *self)
{
  wl_list_remove(&self->views);
  zn_screen_layout_remove(self->screen_layout, self);
  free(self);
}
