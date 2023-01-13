#include <cairo-ft.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <zen-common.h>
#include <zigzag.h>

void
zigzag_layout_add_node(struct zigzag_layout *layout, struct zigzag_node *node,
    struct wlr_renderer *renderer)
{
  zigzag_node_update_frame(node);
  zigzag_node_update_texture(node, renderer);
  wl_list_insert(&layout->node_list, &node->link);
}

void
zigzag_layout_get_text_extents(struct zigzag_layout *self, char *utf8,
    double font_size, cairo_text_extents_t *extents)
{
  cairo_t *cr = cairo_create(self->surface);
  if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
    zn_error("Failed to create cairo_t");
    return;
  }
  cairo_set_font_face(cr, zn_font_face_get_cairo_font_face(ZN_FONT_REGULAR));
  cairo_set_font_size(cr, font_size);
  cairo_text_extents(cr, utf8, extents);
  cairo_destroy(cr);
}

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

  self->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 0, 0);
  if (cairo_surface_status(self->surface) != CAIRO_STATUS_SUCCESS) {
    zn_error("Failed to create cairo_surface");
    goto err_free;
  }

  self->screen_width = screen_width;
  self->screen_height = screen_height;

  self->user_data = user_data;

  self->implementation = implementation;

  wl_list_init(&self->node_list);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zigzag_layout_destroy(struct zigzag_layout *self)
{
  cairo_surface_destroy(self->surface);
  wl_list_remove(&self->node_list);
  free(self);
}
