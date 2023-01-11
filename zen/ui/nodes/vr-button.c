#include "zen/ui/nodes/vr-button.h"

#include <cairo.h>
#include <zen-common.h>
#include <zigzag.h>

#include "zen/server.h"
#include "zen/ui/layout-constants.h"
#include "zen/ui/nodes/vr-menu/vr-menu.h"

static void
zn_vr_button_on_click(struct zigzag_node *node, double x, double y)
{
  UNUSED(x);
  UNUSED(y);
  struct zn_vr_button *vr_button = node->user_data;
  if (vr_button->vr_menu->zigzag_node->visible) {
    zigzag_node_hide(vr_button->vr_menu->zigzag_node);
  } else {
    zigzag_node_show(vr_button->vr_menu->zigzag_node);
  }
}

static bool
zn_vr_icon_render(struct zigzag_node *node, cairo_t *cr)
{
  UNUSED(node);
  bool result = zigzag_cairo_stamp_svg_on_surface(
      cr, VR_ICON_BLUE, 0., 0., vr_icon_width, vr_icon_height);
  return result;
}

static bool
zn_vr_button_render(struct zigzag_node *node, cairo_t *cr)
{
  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.7);
  zigzag_cairo_draw_rounded_rectangle(cr, 0., 0., node->frame.width,
      node->frame.height, node->frame.height / 2);
  cairo_fill_preserve(cr);
  cairo_set_line_width(cr, 0.5);
  cairo_set_source_rgb(cr, 0.07, 0.12, 0.30);
  cairo_stroke(cr);

  double padding = 15.;
  cairo_set_font_size(cr, 13);
  zigzag_cairo_draw_text(cr, "VR", node->frame.width / 2 + 5.,
      node->frame.height / 2, ZIGZAG_ANCHOR_LEFT, ZIGZAG_ANCHOR_CENTER);

  double icon_x = padding;
  double icon_y = (node->frame.height - vr_icon_height) / 2;

  struct zn_vr_button *vr_button = node->user_data;
  cairo_set_source_surface(cr, vr_button->vr_icon_surface, icon_x, icon_y);
  cairo_paint(cr);

  return true;
}

static void
zn_vr_button_set_frame(
    struct zigzag_node *node, double screen_width, double screen_height)
{
  double vr_button_height = menu_bar_height - vr_button_margin_height * 2;

  node->frame.x = screen_width - vr_button_width - vr_button_margin_width;
  node->frame.y = screen_height - vr_button_height - vr_button_margin_height;
  node->frame.width = vr_button_width;
  node->frame.height = vr_button_height;
}

static const struct zigzag_node_impl implementation = {
    .on_click = zn_vr_button_on_click,
    .set_frame = zn_vr_button_set_frame,
    .render = zn_vr_button_render,
};

struct zn_vr_button *
zn_vr_button_create(
    struct zigzag_layout *zigzag_layout, struct wlr_renderer *renderer)
{
  struct zn_vr_button *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  struct zigzag_node *zigzag_node =
      zigzag_node_create(&implementation, zigzag_layout, renderer, true, self);

  if (zigzag_node == NULL) {
    zn_error("Failed to create a zigzag_node");
    goto err_vr_button;
  }
  self->zigzag_node = zigzag_node;

  struct zn_vr_menu *vr_menu = zn_vr_menu_create(zigzag_layout, renderer,
      zigzag_layout->screen_width - vr_button_margin_width -
          vr_button_width / 2);
  if (vr_menu == NULL) {
    zn_error("Failed to create the vr_menu");
    goto err_zigzag_node;
  }
  self->vr_menu = vr_menu;
  wl_list_insert(&self->zigzag_node->node_list, &vr_menu->zigzag_node->link);

  self->vr_icon_surface = zigzag_node_render_cairo_surface(
      zigzag_node, zn_vr_icon_render, vr_icon_width, vr_icon_height);

  if (self->vr_icon_surface == NULL) {
    zn_error("Failed to load the icon");
    goto err_vr_menu;
  }

  zigzag_node_update_texture(self->zigzag_node, renderer);

  return self;

err_vr_menu:
  zn_vr_menu_destroy(vr_menu);

err_zigzag_node:
  zigzag_node_destroy(zigzag_node);

err_vr_button:
  free(self);

err:
  return NULL;
}

void
zn_vr_button_destroy(struct zn_vr_button *self)
{
  cairo_surface_destroy(self->vr_icon_surface);
  zn_vr_menu_destroy(self->vr_menu);
  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
