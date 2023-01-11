#include "zen/ui/zigzag-board-layout.h"

#include <wlr/render/wlr_renderer.h>
#include <zen-common.h>
#include <zigzag.h>

#include "zen/board.h"
#include "zen/screen.h"
#include "zen/ui/nodes/board-node.h"

static void
zn_zigzag_board_layout_on_damage(struct zigzag_node *node)
{
  struct zn_zigzag_board_layout *self = node->layout->user_data;
  zn_screen_damage_force(self->screen, &node->frame);
}

static const struct zigzag_layout_impl implementation = {
    .on_damage = zn_zigzag_board_layout_on_damage,
};

struct zn_zigzag_board_layout *
zn_zigzag_board_layout_create(
    struct zn_screen *screen, struct wlr_renderer *renderer)
{
  struct zn_zigzag_board_layout *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }
  self->screen = screen;

  double screen_width, screen_height;
  zn_screen_get_effective_size(screen, &screen_width, &screen_height);

  struct zigzag_layout *zigzag_layout =
      zigzag_layout_create(&implementation, screen_width, screen_height, self);

  if (zigzag_layout == NULL) {
    zn_error("Failed to create a zigzag_layout");
    goto err_zn_zigzag_board_layout;
  }

  self->zigzag_layout = zigzag_layout;

  self->left = zn_board_node_create(zigzag_layout, true, false);
  zigzag_layout_add_node(
      self->zigzag_layout, self->left->zigzag_node, renderer);

  self->center = zn_board_node_create(zigzag_layout, false, false);
  zigzag_layout_add_node(
      self->zigzag_layout, self->center->zigzag_node, renderer);

  self->right = zn_board_node_create(zigzag_layout, false, true);
  zigzag_layout_add_node(
      self->zigzag_layout, self->right->zigzag_node, renderer);

  zn_zigzag_board_layout_notify_board_changed(self);

  return self;

err_zn_zigzag_board_layout:
  free(self);

err:
  return NULL;
}

void
zn_zigzag_board_layout_destroy(struct zn_zigzag_board_layout *self)
{
  zigzag_layout_destroy(self->zigzag_layout);
  free(self);
}

void
zn_zigzag_board_layout_notify_board_changed(struct zn_zigzag_board_layout *self)
{
  struct zn_board *first =
      zn_container_of(self->screen->board_list.next, first, screen_link);

  if (self->screen->current_board == first) {
    zigzag_node_hide(self->left->zigzag_node);
  } else {
    zigzag_node_show(self->left->zigzag_node);
  }

  struct zn_board *last =
      zn_container_of(self->screen->board_list.prev, last, screen_link);
  if (self->screen->current_board == last) {
    zigzag_node_hide(self->right->zigzag_node);
  } else {
    zigzag_node_show(self->right->zigzag_node);
  }
}
