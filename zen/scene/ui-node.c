#include "zen/scene/ui-node.h"

#include <cairo.h>
#include <drm_fourcc.h>
#include <wayland-server-core.h>

#include "zen-common.h"
#include "zen/cairo/texture.h"

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
vr_button_on_click(struct zn_ui_node *self, double x, double y)
{
  UNUSED(self);
  UNUSED(x);
  UNUSED(y);
  zn_warn("VR Mode starting...");
}

struct zn_ui_node *
create_vr_button(struct zn_server *server, int output_width, int output_height)
{
  struct zn_ui_node *vr_button = NULL;
  double button_width = 160;
  double button_height = 40;
  cairo_surface_t *surface = cairo_image_surface_create(
      CAIRO_FORMAT_RGB24, button_width, button_height);

  if (!zn_assert(cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS,
          "Failed to create cairo_surface"))
    goto err_cairo_surface;

  cairo_t *cr = cairo_create(surface);
  if (!zn_assert(
          cairo_status(cr) == CAIRO_STATUS_SUCCESS, "Failed to create cairo_t"))
    goto err_cairo;

  // Want The corners to be rounded. needs cairo_line_to() and cairo_curve_to
  // With cairo_in_fill(), non-rectangle clickable area can be created
  cairo_set_source_rgb(cr, 0.067, 0.122, 0.302);
  zn_cairo_draw_rounded_rectangle(
      cr, button_width, button_height, button_height / 2);
  cairo_fill(cr);

  cairo_select_font_face(
      cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 18);
  cairo_set_source_rgb(cr, 0.953, 0.957, 0.965);
  cairo_text_extents_t extents;
  cairo_text_extents(cr, "VR", &extents);
  // Implement draw(center)
  cairo_move_to(cr, button_width / 2 - (extents.width / 2 + extents.x_bearing),
      button_height / 2 - (extents.height / 2 + extents.y_bearing));
  cairo_show_text(cr, "VR");

  struct wlr_texture *texture =
      zn_wlr_texture_from_cairo_surface(surface, server);

  struct wlr_box *frame;
  frame = zalloc(sizeof *frame);

  // Put the frame to the center of screen
  frame->x = (double)output_width / 2 - button_width / 2;
  frame->y = (double)output_height - button_height - 10.;
  frame->width = button_width;
  frame->height = button_height;

  vr_button = zn_ui_node_create(frame, texture, vr_button_on_click);

  if (vr_button == NULL) {
    zn_error("Failed to create the VR button");
    goto err_cairo;
  }
err_cairo:
  cairo_destroy(cr);
err_cairo_surface:
  cairo_surface_destroy(surface);
  return vr_button;
}

void
menu_bar_on_click(struct zn_ui_node *self, double x, double y)
{
  UNUSED(self);
  UNUSED(x);
  UNUSED(y);
}

struct zn_ui_node *
create_menu_bar(struct zn_server *server, int output_width, int output_height,
    struct zn_ui_node *vr_button)
{
  struct zn_ui_node *menu_bar = NULL;
  double bar_width = (double)output_width;
  double bar_height = 60;
  cairo_surface_t *surface =
      cairo_image_surface_create(CAIRO_FORMAT_RGB24, bar_width, bar_height);

  if (!zn_assert(cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS,
          "Failed to create cairo_surface"))
    goto err_cairo_surface;

  cairo_t *cr = cairo_create(surface);
  if (!zn_assert(
          cairo_status(cr) == CAIRO_STATUS_SUCCESS, "Failed to create cairo_t"))
    goto err_cairo;

  // Want The corners to be rounded. needs cairo_line_to() and cairo_curve_to
  // With cairo_in_fill(), non-rectangle clickable area can be created
  cairo_set_source_rgb(cr, 0.529, 0.557, 0.647);
  cairo_paint(cr);
  struct wlr_texture *texture =
      zn_wlr_texture_from_cairo_surface(surface, server);

  struct wlr_box *frame;
  frame = zalloc(sizeof *frame);

  // Put the frame to the center of screen
  frame->x = 0.;
  frame->y = (double)output_height - bar_height;
  frame->width = bar_width;
  frame->height = bar_height;

  menu_bar = zn_ui_node_create(frame, texture, menu_bar_on_click);
  wl_list_insert(&menu_bar->children, &vr_button->link);

  if (menu_bar == NULL) {
    zn_error("Failed to create the menu bar");
    goto err_cairo;
  }
err_cairo:
  cairo_destroy(cr);
err_cairo_surface:
  cairo_surface_destroy(surface);
  return menu_bar;
}

void
zn_ui_node_setup_default(struct zn_screen *screen, struct zn_server *server)
{
  int output_width, output_height;
  wlr_output_transformed_resolution(
      screen->output->wlr_output, &output_width, &output_height);
  struct zn_ui_node *vr_button =
      create_vr_button(server, output_width, output_height);
  struct zn_ui_node *menu_bar =
      create_menu_bar(server, output_width, output_height, vr_button);
  // Register the widgets on the screen
  wl_list_insert(&screen->ui_nodes, &menu_bar->link);
}