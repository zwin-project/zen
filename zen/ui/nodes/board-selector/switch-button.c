#include "zen/ui/nodes/board-selector/switch-button.h"

#include <cairo.h>
#include <zen-common.h>
#include <zigzag.h>

#include "zen/board.h"
#include "zen/scene.h"
#include "zen/screen/output.h"
#include "zen/server.h"
#include "zen/ui/layout-constants.h"
#include "zen/ui/nodes/board-selector/board-selector.h"

static void
zn_board_selector_item_switch_button_on_click(
    struct zigzag_node *node, double x, double y)
{
  UNUSED(x);
  UNUSED(y);
  struct zn_board_selector_item_switch_button *self = node->user_data;
  zn_screen_set_current_board(self->parent->screen, self->board);
}

static bool
zn_board_selector_item_switch_button_is_active(
    struct zn_board_selector_item_switch_button *self)
{
  return self->board == self->parent->screen->current_board;
}

static bool
zn_board_selector_item_switch_button_render(
    struct zigzag_node *node, cairo_t *cr)
{
  struct zn_board_selector_item_switch_button *self = node->user_data;

  zigzag_cairo_draw_node_frame(cr, node,
      zn_board_selector_item_switch_button_is_active(self)
          ? (struct zigzag_color){0.07, 0.12, 0.30, 1.0}
          : (struct zigzag_color){0.04, 0.12, 0.30, 0.3},
      (struct zigzag_color){0, 0, 0, 0}, 0, 8.);

  char board_name[8];
  sprintf(board_name, "Board %d", self->index);
  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_set_font_size(cr, 13.);
  zigzag_cairo_draw_text(cr, board_name, node->frame.width / 2,
      node->frame.height / 2, ZIGZAG_ANCHOR_CENTER, ZIGZAG_ANCHOR_CENTER);
  return true;
}

static const struct zigzag_node_impl implementation = {
    .on_click = zn_board_selector_item_switch_button_on_click,
    .render = zn_board_selector_item_switch_button_render,
};

static void
zn_board_selector_item_switch_button_update_frame(struct zigzag_node *node)
{
  struct zn_board_selector_item_switch_button *self = node->user_data;
  node->margin.left = node->margin.right = 8.;
  node->pending.frame.width = board_selector_item_switch_button_width;
  node->pending.frame.height = board_selector_item_switch_button_height;
  node->pending.frame.y = self->parent->zigzag_node->frame.y;
}

static void
zn_board_selector_item_switch_button_handle_board_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_board_selector_item_switch_button *self =
      zn_container_of(listener, self, board_destroy_listener);
  zn_board_selector_item_switch_button_destroy(self);
}

void
zn_board_selector_item_switch_button_update(
    struct zn_board_selector_item_switch_button *self,
    struct wlr_renderer *renderer)
{
  zn_board_selector_item_switch_button_update_frame(self->zigzag_node);
  zigzag_node_update_frame(self->zigzag_node);
  zigzag_node_update_texture(self->zigzag_node, renderer);
}

struct zn_board_selector_item_switch_button *
zn_board_selector_item_switch_button_create(struct zigzag_layout *zigzag_layout,
    struct zn_board *board, struct zn_board_selector *parent, uint32_t index)
{
  struct zn_board_selector_item_switch_button *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zigzag_node =
      zigzag_node_create(&implementation, zigzag_layout, true, self);
  if (self->zigzag_node == NULL) {
    zn_error("Failed to create a zigzag_node");
    goto err_board_selector_item_switch_button;
  }

  self->board = board;
  self->parent = parent;
  self->index = index;
  wl_list_init(&self->link);

  self->board_destroy_listener.notify =
      zn_board_selector_item_switch_button_handle_board_destroy;
  wl_signal_add(&board->events.destroy, &self->board_destroy_listener);

  zn_board_selector_item_switch_button_update_frame(self->zigzag_node);

  return self;

err_board_selector_item_switch_button:
  free(self);

err:
  return NULL;
}

void
zn_board_selector_item_switch_button_destroy(
    struct zn_board_selector_item_switch_button *self)
{
  if (self->parent) {
    zn_board_selector_update(self->parent);
  }

  wl_list_remove(&self->board_destroy_listener.link);
  wl_list_remove(&self->link);
  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
