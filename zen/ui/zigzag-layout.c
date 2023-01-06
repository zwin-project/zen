#include "zen/ui/zigzag-layout.h"

#include <time.h>
#include <wlr/render/wlr_renderer.h>
#include <zen-common.h>
#include <zigzag.h>

#include "zen/screen.h"
#include "zen/server.h"
#include "zen/ui/nodes/menu-bar.h"

static void
zn_zigzag_layout_on_damage(struct zigzag_node *node)
{
  struct zn_zigzag_layout *self = node->layout->user_data;
  zn_screen_damage(self->screen, &node->frame);
}

static const struct zigzag_layout_impl implementation = {
    .on_damage = zn_zigzag_layout_on_damage,
};

struct zn_zigzag_layout *
zn_zigzag_layout_create(struct zn_screen *screen, struct wlr_renderer *renderer)
{
  struct zn_zigzag_layout *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }
  self->screen = screen;

  double screen_width, screen_height;
  zn_screen_get_effective_size(screen, &screen_width, &screen_height);

  struct zigzag_layout *zigzag_layout = zigzag_layout_create(
      &implementation, screen_width, screen_height, DEFAULT_SYSTEM_FONT, self);

  if (zigzag_layout == NULL) {
    zn_error("Failed to create a zigzag_layout");
    goto err_zn_zigzag_layout;
  }

  self->zigzag_layout = zigzag_layout;

  struct zn_menu_bar *menu_bar = zn_menu_bar_create(zigzag_layout, renderer);

  if (menu_bar == NULL) {
    zn_error("Failed to create the menu_bar");
    goto err_zigzag_layout;
  }
  self->menu_bar = menu_bar;

  wl_list_insert(&self->zigzag_layout->node_list, &menu_bar->zigzag_node->link);

  return self;

err_zigzag_layout:
  zigzag_layout_destroy(zigzag_layout);

err_zn_zigzag_layout:
  free(self);

err:
  return NULL;
}

void
zn_zigzag_layout_destroy(struct zn_zigzag_layout *self)
{
  zn_menu_bar_destroy(self->menu_bar);
  zigzag_layout_destroy(self->zigzag_layout);
  free(self);
}

static void
notify_click_recursive(struct wl_list *nodes, double x, double y)
{
  struct zigzag_node *node;
  wl_list_for_each (node, nodes, link) {
    if (zigzag_node_contains_point(node, x, y)) {
      node->implementation->on_click(node, x, y);
    }
    notify_click_recursive(&node->node_list, x, y);
  }
}

void
zn_zigzag_layout_notify_click(struct zn_zigzag_layout *self, double x, double y)
{
  notify_click_recursive(&self->zigzag_layout->node_list, x, y);
}
