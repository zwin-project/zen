#include "zen/ui/nodes/menu-bar.h"

#include <cairo.h>
#include <zen-common.h>
#include <zigzag.h>

#include "zen/screen/output.h"
#include "zen/ui/layout-constants.h"
#include "zen/ui/nodes/app-launcher.h"
#include "zen/ui/nodes/board-selector/board-selector.h"
#include "zen/ui/nodes/power-button.h"
#include "zen/ui/nodes/vr-button.h"

static const struct zn_app_launcher_data default_launchers[2] = {
    {.icon_path = CHROME_LAUNCHER_ICON,
        .command = "google-chrome-stable --enable-features=UseOzonePlatform "
                   "--ozone-platform=wayland --disable-gpu"},
    {.icon_path = TERMINAL_LAUNCHER_ICON, .command = "weston-terminal"},
};

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
  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.3);
  cairo_paint(cr);
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.88);
  cairo_set_line_width(cr, 0.25);
  cairo_rectangle(cr, 0., 0., node->frame.width, node->frame.height);
  cairo_stroke(cr);
  return true;
}

static const struct zigzag_node_impl implementation = {
    .on_click = zn_menu_bar_on_click,
    .render = zn_menu_bar_render,
};

static void
zn_menu_bar_init_frame(
    struct zigzag_node *node, double screen_width, double screen_height)
{
  node->pending.frame.x = 0.;
  node->pending.frame.y = screen_height - menu_bar_height;
  node->pending.frame.width = screen_width;
  node->pending.frame.height = menu_bar_height;
}

struct zn_menu_bar *
zn_menu_bar_create(struct zigzag_layout *zigzag_layout,
    struct wlr_renderer *renderer, struct zn_screen *screen)
{
  struct zn_menu_bar *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  struct zigzag_node *zigzag_node =
      zigzag_node_create(&implementation, zigzag_layout, true, self);

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

  zigzag_node_add_child(self->zigzag_node, power_button->zigzag_node, renderer);

  struct zn_board_selector *board_selector =
      zn_board_selector_create(zigzag_layout, screen);
  if (board_selector == NULL) {
    zn_error("Failed to create the board_selector");
    goto err_power_button;
  }
  self->board_selector = board_selector;

  zigzag_node_add_child(
      self->zigzag_node, board_selector->zigzag_node, renderer);

  zn_menu_bar_init_frame(self->zigzag_node,
      self->zigzag_node->layout->screen_width,
      self->zigzag_node->layout->screen_height);

  struct zn_vr_button *vr_button = zn_vr_button_create(zigzag_layout, renderer);
  if (vr_button == NULL) {
    zn_error("Failed to create the vr_button");
    goto err_power_button;
  }
  self->vr_button = vr_button;

  zigzag_node_add_child(self->zigzag_node, vr_button->zigzag_node, renderer);

  struct zn_app_launcher *launcher, *launcher_tmp;
  wl_list_init(&self->launcher_list);
  for (uint64_t i = 0; i < ARRAY_LENGTH(default_launchers); i++) {
    launcher = zn_app_launcher_create(zigzag_layout, &default_launchers[i], i);
    if (launcher == NULL) {
      zn_error("Failed to create the launcher %ld", i);
      goto err_launcher_list;
    }
    wl_list_insert(&self->launcher_list, &launcher->link);
    zigzag_node_add_child(self->zigzag_node, launcher->zigzag_node, renderer);
  }

  return self;

err_launcher_list:
  wl_list_for_each_reverse_safe (
      launcher, launcher_tmp, &self->launcher_list, link) {
    zn_app_launcher_destroy(launcher);
  }
  wl_list_remove(&self->launcher_list);

  zn_vr_button_destroy(vr_button);

err_power_button:
  zn_power_button_destroy(power_button);

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
  struct zn_app_launcher *launcher, *tmp;
  wl_list_for_each_reverse_safe (launcher, tmp, &self->launcher_list, link) {
    zn_app_launcher_destroy(launcher);
  }

  wl_list_remove(&self->launcher_list);
  zn_vr_button_destroy(self->vr_button);
  zn_board_selector_destroy(self->board_selector);
  zn_power_button_destroy(self->power_button);
  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
