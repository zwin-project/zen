#include "zen/ui/nodes/board-selector/board-selector.h"

#include <cairo.h>
#include <zen-common.h>
#include <zigzag.h>

#include "zen/board.h"
#include "zen/scene.h"
#include "zen/screen/output.h"
#include "zen/server.h"
#include "zen/ui/layout-constants.h"
#include "zen/ui/nodes/board-selector/switch-button.h"

static void
zn_board_selector_on_click(struct zigzag_node *node, double x, double y)
{
  UNUSED(node);
  UNUSED(x);
  UNUSED(y);
}

static bool
zn_board_selector_render(struct zigzag_node *node, cairo_t *cr)
{
  UNUSED(node);
  UNUSED(cr);
  return true;
}

static const struct zigzag_node_impl implementation = {
    .on_click = zn_board_selector_on_click,
    .render = zn_board_selector_render,
};

static void
zn_board_selector_update_frame(struct zigzag_node *node)
{
  double width, height;
  zigzag_node_child_total_size(node, &width, &height);
  node->pending.frame.width = fmax(width, 1);
  node->pending.frame.height = board_selector_item_switch_button_height;
  node->pending.frame.x = node->layout->screen_width / 2 - width / 2;
  node->pending.frame.y = node->layout->screen_height - menu_bar_height / 2 -
                          board_selector_item_switch_button_height / 2;
}

void
zn_board_selector_update(struct zn_board_selector *self)
{
  struct zn_server *server = zn_server_get_singleton();

  struct zn_board_selector_item_switch_button *button;
  wl_list_for_each (
      button, &self->board_selector_item_switch_button_list, link) {
    zn_board_selector_item_switch_button_update(button, server->renderer);
  }

  zn_board_selector_update_frame(self->zigzag_node);
  zigzag_node_update_frame(self->zigzag_node);
  zigzag_node_reconfigure(self->zigzag_node, ZIGZAG_RECONFIGURE_HORIZONTAL,
      ZIGZAG_RECONFIGURE_START);
}

static void
zn_board_selector_add_button(
    struct zn_board_selector *self, struct zn_board *board)
{
  struct zn_board_selector_item_switch_button
      *board_selector_item_switch_button =
          zn_board_selector_item_switch_button_create(self->zigzag_node->layout,
              board, self,
              wl_list_length(&self->board_selector_item_switch_button_list) +
                  1);
  if (!board_selector_item_switch_button) {
    return;
  }
  wl_list_insert(&self->board_selector_item_switch_button_list,
      &board_selector_item_switch_button->link);

  struct zn_server *server = zn_server_get_singleton();
  zigzag_node_add_child(self->zigzag_node,
      board_selector_item_switch_button->zigzag_node, server->renderer);
}

static void
zn_board_selector_handle_board_mapped_to_screen(
    struct wl_listener *listener, void *data)
{
  struct zn_board_selector *self =
      zn_container_of(listener, self, board_mapped_to_screen_listener);
  struct zn_board *board = data;

  if (board->screen == self->screen) {
    zn_board_selector_add_button(self, board);
    zn_board_selector_update(self);
  }
}

static void
zn_board_selector_handle_screen_current_board_changed(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_board_selector *self =
      zn_container_of(listener, self, screen_current_board_changed_listener);
  struct zn_server *server = zn_server_get_singleton();

  struct zn_board_selector_item_switch_button *button;
  wl_list_for_each (
      button, &self->board_selector_item_switch_button_list, link) {
    zn_board_selector_item_switch_button_update(button, server->renderer);
  }

  zn_board_selector_update(self);
}

struct zn_board_selector *
zn_board_selector_create(
    struct zigzag_layout *zigzag_layout, struct zn_screen *screen)
{
  struct zn_board_selector *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  struct zigzag_node *zigzag_node =
      zigzag_node_create(&implementation, zigzag_layout, true, self);

  if (zigzag_node == NULL) {
    zn_error("Failed to create a zigzag_node");
    goto err_board_selector;
  }
  self->zigzag_node = zigzag_node;

  self->screen = screen;

  struct zn_server *server = zn_server_get_singleton();
  self->board_mapped_to_screen_listener.notify =
      zn_board_selector_handle_board_mapped_to_screen;
  wl_signal_add(&server->scene->events.board_mapped_to_screen,
      &self->board_mapped_to_screen_listener);

  self->screen_current_board_changed_listener.notify =
      zn_board_selector_handle_screen_current_board_changed;
  wl_signal_add(&screen->events.current_board_changed,
      &self->screen_current_board_changed_listener);

  wl_list_init(&self->board_selector_item_switch_button_list);

  zn_board_selector_update_frame(self->zigzag_node);
  zigzag_node_update_frame(self->zigzag_node);

  return self;

err_board_selector:
  free(self);

err:
  return NULL;
}

void
zn_board_selector_destroy(struct zn_board_selector *self)
{
  wl_list_remove(&self->board_mapped_to_screen_listener.link);
  wl_list_remove(&self->screen_current_board_changed_listener.link);

  struct zn_board_selector_item_switch_button *button, *tmp;
  wl_list_for_each_safe (
      button, tmp, &self->board_selector_item_switch_button_list, link) {
    button->parent = NULL;
    zn_board_selector_item_switch_button_destroy(button);
  }

  zigzag_node_destroy(self->zigzag_node);
  free(self);
}
