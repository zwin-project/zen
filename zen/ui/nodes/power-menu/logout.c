#include "zen/ui/nodes/power-menu/logout.h"

#include <cairo.h>
#include <zen-common.h>

#include "zen/server.h"
#include "zen/ui/layout-constants.h"
#include "zigzag/cairo-util.h"
#include "zigzag/layout.h"
#include "zigzag/node.h"

static void
zn_power_menu_item_logout_on_click(struct zigzag_node *self, double x, double y)
{
  UNUSED(self);
  UNUSED(x);
  UNUSED(y);
  zn_terminate(EXIT_SUCCESS);
}

static bool
zn_power_menu_item_logout_render(struct zigzag_node *node, cairo_t *cr)
{
  double gray_line_margin = 10.;
  cairo_set_source_rgba(cr, 0., 0., 0., 0.12);
  cairo_move_to(cr, gray_line_margin, 0);
  cairo_line_to(cr, node->frame.width - gray_line_margin, 0);
  cairo_stroke(cr);

  cairo_set_source_rgb(cr, 0., 0., 0.);
  cairo_set_font_size(cr, 13);
  zigzag_cairo_draw_text(cr, "Log out", gray_line_margin * 2,
      node->frame.height / 2, ZIGZAG_ANCHOR_LEFT, ZIGZAG_ANCHOR_CENTER);

  return true;
}

static const struct zigzag_node_impl implementation = {
    .on_click = zn_power_menu_item_logout_on_click,
    .render = zn_power_menu_item_logout_render,
};

static void
zn_power_menu_item_logout_init_frame(
    struct zigzag_node *node, double screen_width, double screen_height)
{
  node->pending.frame.x =
      screen_width - power_menu_bubble_width - power_menu_space_right;
  node->pending.frame.y = screen_height - power_menu_bubble_height -
                          menu_bar_height + power_menu_clock_height + 5.;

  node->pending.frame.width = power_menu_bubble_width;
  node->pending.frame.height = power_menu_logout_height;
}

struct zn_power_menu_item_logout *
zn_power_menu_item_logout_create(
    struct zigzag_layout *zigzag_layout, struct wlr_renderer *renderer)
{
  UNUSED(renderer);
  struct zn_power_menu_item_logout *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  struct zigzag_node *zigzag_node =
      zigzag_node_create(&implementation, zigzag_layout, true, self);

  if (zigzag_node == NULL) {
    zn_error("Failed to create a zigzag_node");
    goto err_power_menu_item_logout;
  }
  self->zigzag_node = zigzag_node;

  zn_power_menu_item_logout_init_frame(self->zigzag_node,
      self->zigzag_node->layout->screen_width,
      self->zigzag_node->layout->screen_height);

  return self;

err_power_menu_item_logout:
  free(self);

err:
  return NULL;
}

void
zn_power_menu_item_logout_destroy(struct zn_power_menu_item_logout *self)
{
  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
