#include "zen/ui/nodes/power-button.h"

#include <cairo.h>
#include <zen-common.h>
#include <zigzag.h>

#include "zen/server.h"
#include "zen/ui/layout-constants.h"

static int
zn_power_button_handle_second_timer(void *data)
{
  struct zn_power_button *self = data;
  struct zigzag_node *zigzag_node = self->zigzag_node;
  struct zn_server *server = zn_server_get_singleton();

  int64_t time_ms = current_realtime_clock_ms();

  int delay_ms = MSEC_PER_SEC - time_ms % MSEC_PER_SEC;
  if (delay_ms <= 0) delay_ms = 1;
  wl_event_source_timer_update(self->second_timer_source, delay_ms);

  zigzag_node_update_texture(zigzag_node, server->renderer);
  return 0;
}

static void
zn_power_button_on_click(struct zigzag_node *node, double x, double y)
{
  UNUSED(node);
  UNUSED(x);
  UNUSED(y);
  struct zn_power_button *power_button = node->user_data;
  if (power_button->power_menu->zigzag_node->visible) {
    zigzag_node_hide(power_button->power_menu->zigzag_node);
  } else {
    zigzag_node_show(power_button->power_menu->zigzag_node);
  }
}

static bool
zn_power_icon_render(struct zigzag_node *node, cairo_t *cr)
{
  UNUSED(node);
  bool result = zigzag_cairo_stamp_svg_on_surface(
      cr, POWER_BUTTON_ICON, 0., 0., power_icon_width, power_icon_height);
  return result;
}

static bool
zn_power_button_render(struct zigzag_node *node, cairo_t *cr)
{
  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.7);
  zigzag_cairo_draw_rounded_rectangle(cr, 0., 0., node->frame.width,
      node->frame.height, node->frame.height / 2);
  cairo_fill_preserve(cr);
  cairo_set_line_width(cr, 0.5);
  cairo_set_source_rgb(cr, 0.07, 0.12, 0.30);
  cairo_stroke(cr);

  time_t raw_time;
  struct tm *time_info;

  char output[6];

  time(&raw_time);
  time_info = localtime(&raw_time);

  sprintf(output, "%02d:%02d", time_info->tm_hour, time_info->tm_min);

  double padding = 6.;
  cairo_set_font_size(cr, 11);
  zigzag_cairo_draw_text(cr, output, padding, node->frame.height / 2,
      ZIGZAG_ANCHOR_LEFT, ZIGZAG_ANCHOR_CENTER);

  struct zn_power_button *power_button = node->user_data;

  double icon_x = node->frame.width - padding - power_icon_width;
  double icon_y = (node->frame.height - power_icon_height) / 2;
  cairo_set_source_surface(
      cr, power_button->power_icon_surface, icon_x, icon_y);
  cairo_paint(cr);

  return true;
}

static void
zn_power_button_set_frame(
    struct zigzag_node *node, double screen_width, double screen_height)
{
  double power_button_height = menu_bar_height - power_button_margin_height * 2;

  node->frame.x = screen_width - power_button_width - power_button_margin_width;
  node->frame.y =
      screen_height - power_button_height - power_button_margin_height;
  node->frame.width = power_button_width;
  node->frame.height = power_button_height;
}

static const struct zigzag_node_impl implementation = {
    .on_click = zn_power_button_on_click,
    .set_frame = zn_power_button_set_frame,
    .render = zn_power_button_render,
};

struct zn_power_button *
zn_power_button_create(
    struct zigzag_layout *zigzag_layout, struct wlr_renderer *renderer)
{
  struct zn_power_button *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  struct zigzag_node *zigzag_node =
      zigzag_node_create(&implementation, zigzag_layout, renderer, true, self);

  if (zigzag_node == NULL) {
    zn_error("Failed to create a zigzag_node");
    goto err_power_button;
  }
  self->zigzag_node = zigzag_node;

  struct zn_power_menu *power_menu =
      zn_power_menu_create(zigzag_layout, renderer,
          zigzag_layout->screen_width - power_button_margin_width -
              power_button_width / 2);
  if (power_menu == NULL) {
    zn_error("Failed to create the power_menu");
    goto err_zigzag_node;
  }
  self->power_menu = power_menu;

  wl_list_insert(&self->zigzag_node->node_list, &power_menu->zigzag_node->link);

  self->power_icon_surface = zigzag_node_render_cairo_surface(
      zigzag_node, zn_power_icon_render, power_icon_width, power_icon_height);

  if (self->power_icon_surface == NULL) {
    zn_error("Failed to load the icon");
    goto err_power_menu;
  }

  struct zn_server *server = zn_server_get_singleton();
  self->second_timer_source = wl_event_loop_add_timer(
      server->loop, zn_power_button_handle_second_timer, self);

  int64_t time_ms = current_realtime_clock_ms();

  int delay_ms = MSEC_PER_SEC - time_ms % MSEC_PER_SEC;
  if (delay_ms <= 0) delay_ms = 1;
  wl_event_source_timer_update(self->second_timer_source, delay_ms);

  return self;

err_power_menu:
  zn_power_menu_destroy(self->power_menu);

err_zigzag_node:
  zigzag_node_destroy(zigzag_node);

err_power_button:
  free(self);

err:
  return NULL;
}

void
zn_power_button_destroy(struct zn_power_button *self)
{
  wl_event_source_remove(self->second_timer_source);
  cairo_surface_destroy(self->power_icon_surface);
  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
