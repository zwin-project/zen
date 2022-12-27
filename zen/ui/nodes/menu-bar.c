#include "zen/ui/nodes/menu-bar.h"

#include <cairo.h>
#include <zen-common.h>
#include <zigzag.h>

#include "zen/screen/output.h"
#include "zen/ui/nodes/power-button.h"

static void
zn_menu_bar_on_click(struct zigzag_node *node, double x, double y)
{
  UNUSED(node);
  UNUSED(x);
  UNUSED(y);
}

static bool
zn_menu_bar_render(struct zigzag_node *node, cairo_t *cr)
{
  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.1);
  cairo_paint(cr);
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.88);
  cairo_set_line_width(cr, 0.25);
  cairo_rectangle(cr, 0., 0., node->frame.width, node->frame.height);
  cairo_stroke(cr);
  return true;
}

static void
zn_menu_bar_set_frame(
    struct zigzag_node *node, double screen_width, double screen_height)
{
  double height = 33.;
  double bar_width = screen_width;

  node->frame.x = 0.;
  node->frame.y = screen_height - height;
  node->frame.width = bar_width;
  node->frame.height = height;
}

static const struct zigzag_node_impl implementation = {
    .on_click = zn_menu_bar_on_click,
    .set_frame = zn_menu_bar_set_frame,
    .render = zn_menu_bar_render,
};

struct zn_menu_bar *
zn_menu_bar_create(
    struct zigzag_layout *zigzag_layout, struct wlr_renderer *renderer)
{
  struct zn_menu_bar *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  struct zigzag_node *zigzag_node =
      zigzag_node_create(&implementation, zigzag_layout, renderer, true, self);

  if (zigzag_node == NULL) {
    zn_error("Failed to create a zigzag_node");
    goto err_menu_bar;
  }
  self->zigzag_node = zigzag_node;

  struct zn_power_button *power_button =
      zn_power_button_create(zigzag_layout, renderer);
  if (power_button == NULL) {
    zn_error("Failed to create the power_button");
    goto err_zigzag_node;
  }
  self->power_button = power_button;

  wl_list_insert(
      &self->zigzag_node->node_list, &power_button->zigzag_node->link);

  return self;

err_zigzag_node:
  zigzag_node_destroy(zigzag_node);

err_menu_bar:
  free(self);

err:
  return NULL;
}

void
zn_menu_bar_destroy(struct zn_menu_bar *self)
{
  zn_power_button_destroy(self->power_button);
  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
