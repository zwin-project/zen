#include "zen/ui/nodes/power-menu.h"

#include <cairo.h>
#include <zen-common.h>
#include <zigzag.h>

#include "zen/server.h"
#include "zen/ui/nodes/power-menu-item-clock.h"
#include "zen/ui/nodes/power-menu-item-logout.h"

static void
zn_power_menu_on_click(struct zigzag_node *self, double x, double y)
{
  UNUSED(self);
  UNUSED(x);
  UNUSED(y);
}

static bool
zn_power_menu_render(struct zigzag_node *self, cairo_t *cr)
{
  struct zn_power_menu *power_menu = (struct zn_power_menu *)self->user_data;
  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  zigzag_cairo_draw_rounded_bubble(cr, 0., 0., self->frame.width,
      self->frame.height, 5., power_menu->tip_x - self->frame.x);
  cairo_fill_preserve(cr);
  cairo_set_line_width(cr, 0.5);
  cairo_set_source_rgba(cr, 0., 0., 0., 0.12);
  cairo_stroke(cr);
  return true;
}

static void
zn_power_menu_set_frame(
    struct zigzag_node *self, double screen_width, double screen_height)
{
  double menu_bar_height = 33.;
  double space_right = 20.;
  double bubble_height = 70.;
  double bubble_width = 110.;

  self->frame.x = screen_width - bubble_width - space_right;
  self->frame.y = screen_height - bubble_height - menu_bar_height;
  self->frame.width = bubble_width;
  self->frame.height = bubble_height;
}

static const struct zigzag_node_impl implementation = {
    .on_click = zn_power_menu_on_click,
    .set_frame = zn_power_menu_set_frame,
    .render = zn_power_menu_render,
};

struct zn_power_menu *
zn_power_menu_create(struct zigzag_layout *zigzag_layout,
    struct wlr_renderer *renderer, double tip_x)
{
  struct zn_power_menu *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }
  self->tip_x = tip_x;

  struct zigzag_node *zigzag_node =
      zigzag_node_create(&implementation, zigzag_layout, renderer, false, self);

  if (zigzag_node == NULL) {
    zn_error("Failed to create a zigzag_node");
    goto err_power_menu;
  }
  self->zigzag_node = zigzag_node;

  struct zn_power_menu_item_clock *power_menu_item_clock =
      zn_power_menu_item_clock_create(zigzag_layout, renderer);
  if (power_menu_item_clock == NULL) {
    zn_error("Failed to create the power_menu_item_clock");
    goto err_zigzag_node;
  }
  self->item_clock = power_menu_item_clock;

  wl_list_insert(
      &self->zigzag_node->node_list, &power_menu_item_clock->zigzag_node->link);

  struct zn_power_menu_item_logout *power_menu_item_logout =
      zn_power_menu_item_logout_create(zigzag_layout, renderer);
  if (power_menu_item_logout == NULL) {
    zn_error("Failed to create the power_menu_item_logout");
    goto err_item_clock;
  }
  self->item_logout = power_menu_item_logout;

  wl_list_insert(&self->zigzag_node->node_list,
      &power_menu_item_logout->zigzag_node->link);

  return self;

err_item_clock:
  zn_power_menu_item_clock_destroy(self->item_clock);

err_zigzag_node:
  zigzag_node_destroy(self->zigzag_node);

err_power_menu:
  free(self);

err:
  return NULL;
}

void
zn_power_menu_destroy(struct zn_power_menu *self)
{
  zn_power_menu_item_logout_destroy(self->item_logout);
  zn_power_menu_item_clock_destroy(self->item_clock);
  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
