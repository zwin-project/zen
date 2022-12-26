#include <stdlib.h>
#include <wayland-server-core.h>
#include <zen-common.h>
#include <zigzag.h>

struct zigzag_layout *
zigzag_layout_create(const struct zigzag_layout_impl *implementation,
    int output_width, int output_height, void *state)
{
  struct zigzag_layout *self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }
  self->output_width = output_width;
  self->output_height = output_height;

  self->state = state;

  self->implementation = implementation;

  wl_list_init(&self->nodes);

  return self;
err:
  return NULL;
}

void
zigzag_layout_destroy(struct zigzag_layout *self)
{
  wl_list_remove(&self->nodes);
  free(self);
}
