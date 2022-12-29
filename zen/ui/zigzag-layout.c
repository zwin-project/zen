#include "zen/ui/zigzag-layout.h"

#include <time.h>
#include <wlr/render/wlr_renderer.h>
#include <zen-common.h>
#include <zigzag.h>

#include "zen/server.h"
#include "zen/ui/nodes/menu-bar.h"

static void
zn_zigzag_layout_on_damage(struct zigzag_node *node)
{
  struct zn_zigzag_layout *self =
      (struct zn_zigzag_layout *)node->layout->user_data;
  wlr_output_damage_add_box(self->damage, node->frame);
}

static const struct zigzag_layout_impl implementation = {
    .on_damage = zn_zigzag_layout_on_damage,
};

struct zn_zigzag_layout *
zn_zigzag_layout_create(struct wlr_output *output,
    struct wlr_renderer *renderer, struct wlr_output_damage *damage)
{
  struct zn_zigzag_layout *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->damage = damage;

  int output_width, output_height;
  wlr_output_transformed_resolution(output, &output_width, &output_height);

  struct zigzag_layout *zigzag_layout = zigzag_layout_create(
      &implementation, output_width, output_height, DEFAULT_SYSTEM_FONT, self);

  if (zigzag_layout == NULL) {
    zn_error("Failed to create a zigzag_layout");
    goto err_zn_zigzag_layout;
  }

  self->zigzag_layout = zigzag_layout;

  struct zn_menu_bar *menu_bar =
      zn_menu_bar_create(zigzag_layout, renderer, output->display);

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
