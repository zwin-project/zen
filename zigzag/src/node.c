#include <cairo.h>
#include <stdlib.h>
#include <wlr/render/wlr_renderer.h>
#include <zen-common.h>
#include <zigzag.h>

cairo_surface_t *
zigzag_node_render_cairo_surface(struct zigzag_node *self,
    zigzag_node_render_t render, double width, double height)
{
  cairo_surface_t *surface =
      cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);

  if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
    zn_error("Failed to create cairo_surface");
    goto err_cairo_surface;
  }

  cairo_t *cr = cairo_create(surface);
  if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
    zn_error("Failed to create cairo_t");
    goto err_cairo;
  }

  cairo_set_font_face(cr, self->layout->system_font);
  bool success = render(self, cr);
  if (success) {
    cairo_destroy(cr);
    return surface;
  }

err_cairo:
  cairo_destroy(cr);
err_cairo_surface:
  cairo_surface_destroy(surface);

  return NULL;
}

static struct wlr_texture *
zigzag_node_render_texture(
    struct zigzag_node *self, struct wlr_renderer *renderer)
{
  cairo_surface_t *surface = zigzag_node_render_cairo_surface(self,
      self->implementation->render, self->frame.width, self->frame.height);
  if (surface == NULL) {
    zn_error("Failed to create a cairo_surface");
    return NULL;
  }

  struct wlr_texture *texture =
      zigzag_wlr_texture_from_cairo_surface(surface, renderer);
  cairo_surface_destroy(surface);
  return texture;
}

void
zigzag_node_update_texture(
    struct zigzag_node *self, struct wlr_renderer *renderer)
{
  struct wlr_texture *original = self->texture;
  self->texture = zigzag_node_render_texture(self, renderer);
  if (self->texture == NULL) {
    zn_error("Failed to update the texture");
    self->texture = original;
    return;
  }
  wlr_texture_destroy(original);
  self->layout->implementation->on_damage(self);
}

struct zigzag_node *
zigzag_node_create(const struct zigzag_node_impl *implementation,
    struct zigzag_layout *layout, struct wlr_renderer *renderer,
    void *user_data)
{
  struct zigzag_node *self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->layout = layout;
  self->user_data = user_data;
  self->implementation = implementation;

  self->implementation->set_frame(
      self, layout->screen_width, layout->screen_height);

  self->texture = zigzag_node_render_texture(self, renderer);
  if (self->texture == NULL) {
    zn_error("Failed to render the texture");
    goto err_self;
  }

  wl_list_init(&self->node_list);
  wl_list_init(&self->link);

  return self;

err_self:
  zigzag_node_destroy(self);

err:
  return NULL;
}

void
zigzag_node_destroy(struct zigzag_node *self)
{
  wl_list_remove(&self->node_list);
  wl_list_remove(&self->link);
  wlr_texture_destroy(self->texture);
  free(self);
}
