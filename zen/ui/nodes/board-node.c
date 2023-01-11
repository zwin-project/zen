#include "zen/ui/nodes/board-node.h"

#include <cairo.h>
#include <zen-common.h>
#include <zigzag.h>

#include "zen/server.h"
#include "zen/ui/layout-constants.h"
#include "zen/ui/nodes/power-menu/clock.h"
#include "zen/ui/nodes/power-menu/logout.h"

static void
zn_board_node_on_click(struct zigzag_node *node, double x, double y)
{
  UNUSED(node);
  UNUSED(x);
  UNUSED(y);
}

static bool
zn_board_node_render(struct zigzag_node *node, cairo_t *cr)
{
  double margin = 40.;
  zigzag_cairo_draw_rounded_rectangle(
      cr, 0., 0., node->frame.width, node->frame.height, 10.);
  cairo_set_source_rgba(cr, 1., 1., 1., 0.2);
  cairo_fill_preserve(cr);

  cairo_set_source_rgba(cr, 0., 0., 0., 1.);
  cairo_set_line_width(cr, 1.);
  cairo_stroke(cr);

  cairo_set_source_rgba(cr, 0., 0., 0., .1);
  double dots_interval = 60.;
  int num_intervals_x =
      (((int)node->frame.width) - ((int)2 * margin)) / ((int)dots_interval);

  int num_intervals_y =
      (((int)node->frame.height) - ((int)2 * margin)) / ((int)dots_interval);

  double margin_x = (node->frame.width - dots_interval * num_intervals_x) / 2;
  double margin_y = (node->frame.height - dots_interval * num_intervals_y) / 2;

  for (int i = 0; i <= num_intervals_x; i++) {
    for (int j = 0; j <= num_intervals_y; j++) {
      cairo_arc(cr, margin_x + dots_interval * i, margin_y + dots_interval * j,
          2., 0., 2 * M_PI);
      cairo_fill(cr);
    }
  }
  return true;
}

static const struct zigzag_node_impl implementation = {
    .on_click = zn_board_node_on_click,
    .render = zn_board_node_render,
};

static void
zn_board_node_init_frame(
    struct zigzag_node *node, double screen_width, double screen_height)
{
  node->pending.frame.width = screen_width - 60.;
  node->pending.frame.height = screen_height - menu_bar_height - 10.;
  node->pending.frame.y = 5.;

  struct zn_board_node *self = node->user_data;
  if (self->left) {
    node->pending.frame.x = -screen_width + 80.;
  } else if (self->right) {
    node->pending.frame.x = screen_width - 20.;
  } else {
    node->pending.frame.x = 30.;
  }
}

struct zn_board_node *
zn_board_node_create(struct zigzag_layout *zigzag_layout, bool left, bool right)
{
  struct zn_board_node *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }
  self->left = left;
  self->right = right;

  struct zigzag_node *zigzag_node =
      zigzag_node_create(&implementation, zigzag_layout, true, self);

  if (zigzag_node == NULL) {
    zn_error("Failed to create a zigzag_node");
    goto err_board_node;
  }
  self->zigzag_node = zigzag_node;
  zn_board_node_init_frame(self->zigzag_node,
      self->zigzag_node->layout->screen_width,
      self->zigzag_node->layout->screen_height);

  return self;

err_board_node:
  free(self);

err:
  return NULL;
}

void
zn_board_node_destroy(struct zn_board_node *self)
{
  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
