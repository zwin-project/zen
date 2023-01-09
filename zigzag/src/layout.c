#include <cairo-ft.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <zen-common.h>
#include <zigzag.h>

struct zigzag_layout *
zigzag_layout_create(const struct zigzag_layout_impl *implementation,
    double screen_width, double screen_height, void *user_data)
{
  struct zigzag_layout *self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }
  self->screen_width = screen_width;
  self->screen_height = screen_height;

  self->user_data = user_data;

  self->implementation = implementation;

  wl_list_init(&self->node_list);

  return self;

err:
  return NULL;
}

void
zigzag_layout_destroy(struct zigzag_layout *self)
{
  wl_list_remove(&self->node_list);
  free(self);
}
