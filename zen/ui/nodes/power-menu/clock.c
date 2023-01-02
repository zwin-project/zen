#include "zen/ui/nodes/power-menu/clock.h"

#include <cairo.h>
#include <zen-common.h>
#include <zigzag.h>

#include "zen/server.h"
#include "zen/ui/layout-constants.h"

static int
zn_power_menu_item_clock_handle_second_timer(void *data)
{
  struct zn_power_menu_item_clock *self = data;
  struct zigzag_node *zigzag_node = self->zigzag_node;
  struct zn_server *server = zn_server_get_singleton();

  int64_t time_ms = current_realtime_clock_ms();

  int ms_delay = MSEC_PER_SEC - time_ms % MSEC_PER_SEC;
  if (ms_delay <= 0) ms_delay = 1;
  wl_event_source_timer_update(self->second_timer_source, ms_delay);

  zigzag_node_update_texture(zigzag_node, server->renderer);
  return 0;
}

static void
zn_power_menu_item_clock_on_click(struct zigzag_node *node, double x, double y)
{
  UNUSED(node);
  UNUSED(x);
  UNUSED(y);
}

static bool
zn_power_menu_item_clock_render(struct zigzag_node *node, cairo_t *cr)
{
  time_t raw_time;
  struct tm *time_info;

  char output[6];

  time(&raw_time);
  time_info = localtime(&raw_time);

  sprintf(output, "%02d:%02d", time_info->tm_hour, time_info->tm_min);

  cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
  cairo_set_font_size(cr, 13);
  zigzag_cairo_draw_text(cr, output, node->frame.width / 2,
      node->frame.height / 2, ZIGZAG_ANCHOR_CENTER, ZIGZAG_ANCHOR_CENTER);
  return true;
}

static void
zn_power_menu_item_clock_set_frame(
    struct zigzag_node *node, double screen_width, double screen_height)
{
  node->frame.x =
      screen_width - power_menu_bubble_width - power_menu_space_right;
  node->frame.y =
      screen_height - power_menu_bubble_height - menu_bar_height + 5.;
  node->frame.width = power_menu_bubble_width;
  node->frame.height = power_menu_clock_height;
}

static const struct zigzag_node_impl implementation = {
    .on_click = zn_power_menu_item_clock_on_click,
    .set_frame = zn_power_menu_item_clock_set_frame,
    .render = zn_power_menu_item_clock_render,
};

struct zn_power_menu_item_clock *
zn_power_menu_item_clock_create(
    struct zigzag_layout *zigzag_layout, struct wlr_renderer *renderer)
{
  struct zn_power_menu_item_clock *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  struct zigzag_node *zigzag_node =
      zigzag_node_create(&implementation, zigzag_layout, renderer, true, self);

  if (zigzag_node == NULL) {
    zn_error("Failed to create a zigzag_node");
    goto err_power_menu_item_clock;
  }
  self->zigzag_node = zigzag_node;

  struct zn_server *server = zn_server_get_singleton();
  self->second_timer_source = wl_event_loop_add_timer(
      server->loop, zn_power_menu_item_clock_handle_second_timer, self);

  int64_t time_ms = current_realtime_clock_ms();

  int ms_delay = MSEC_PER_SEC - time_ms % MSEC_PER_SEC;
  if (ms_delay <= 0) ms_delay = 1;
  wl_event_source_timer_update(self->second_timer_source, ms_delay);

  return self;

err_power_menu_item_clock:
  free(self);

err:
  return NULL;
}

void
zn_power_menu_item_clock_destroy(struct zn_power_menu_item_clock *self)
{
  wl_event_source_remove(self->second_timer_source);
  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
