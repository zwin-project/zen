#include "zen/scene/ui-node.h"

#include <cairo.h>
#include <drm_fourcc.h>
#include <wayland-server-core.h>

#include "zen-common.h"
#include "zen/cairo/texture.h"

void
vr_button_on_click(struct zn_ui_node *self, double x, double y)
{
  UNUSED(self);
  UNUSED(x);
  UNUSED(y);
  zn_warn("VR Mode starting...");
}

struct zn_ui_node *
zn_ui_node_create(struct wlr_box *frame, struct wlr_texture *texture,
    zn_ui_node_on_click_handler_t handler)
{
  struct zn_ui_node *self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }
  wl_list_init(&self->children);
  self->frame = frame;
  self->texture = texture;
  self->handler = handler;
err:
  return self;
}

void
zn_ui_node_destroy(struct zn_ui_node *self)
{
  wl_list_remove(&self->children);
  wlr_texture_destroy(self->texture);
  free(self->frame);
  free(self);
}

void
zn_ui_node_setup_default(struct zn_screen *screen, struct zn_server *server)
{
  // VR button
  // Want to change the color, shape...etc
  cairo_surface_t *surface =
      cairo_image_surface_create(CAIRO_FORMAT_RGB24, 200, 100);

  if (!zn_assert(cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS,
          "Failed to create cairo_surface"))
    goto err_cairo_surface;

  cairo_t *cr = cairo_create(surface);
  if (!zn_assert(
          cairo_status(cr) == CAIRO_STATUS_SUCCESS, "Failed to create cairo_t"))
    goto err_cairo;

  cairo_set_source_rgb(cr, 1., 1., 1.);
  cairo_paint(cr);

  cairo_select_font_face(
      cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 13);
  cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
  cairo_move_to(cr, 20, 30);
  cairo_show_text(cr, "VR");

  struct wlr_texture *texture =
      zn_wlr_texture_from_cairo_surface(surface, server);

  struct wlr_box *frame;
  frame = zalloc(sizeof *frame);

  frame->x = 0.;
  frame->y = 0.;
  frame->width = 200.;
  frame->height = 100.;

  struct zn_ui_node *vr_button =
      zn_ui_node_create(frame, texture, vr_button_on_click);

  if (vr_button == NULL) {
    zn_error("Failed to create the VR button");
    goto err_cairo;
  }

  // Register the widgets on the screen
  wl_list_insert(&screen->ui_nodes, &vr_button->link);

err_cairo:
  cairo_destroy(cr);
err_cairo_surface:
  cairo_surface_destroy(surface);
  return;
}