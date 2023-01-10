#include "zen/screen.h"

#include <zen-common.h>

#include "zen/board.h"
#include "zen/cursor.h"
#include "zen/scene.h"
#include "zen/screen-layout.h"
#include "zen/server.h"
#include "zen/ui/zigzag-layout.h"
#include "zen/view.h"

void
zn_screen_handle_current_board_destroy(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_screen *self =
      zn_container_of(listener, self, current_board_destroy_listener);

  struct zn_board *board = NULL;
  if (wl_list_length(&self->board_list) != 0) {
    board = zn_container_of(self->board_list.next, board, screen_link);
  }
  zn_screen_set_current_board(self, board);
}

void
zn_screen_damage_force(struct zn_screen *self, struct wlr_fbox *box)
{
  self->implementation->damage(self->user_data, box);
}

void
zn_screen_damage(struct zn_screen *self, struct wlr_fbox *box)
{
  struct zn_server *server = zn_server_get_singleton();
  if (server->display_system != ZN_DISPLAY_SYSTEM_SCREEN) return;

  zn_screen_damage_force(self, box);
}

void
zn_screen_damage_whole(struct zn_screen *self)
{
  struct zn_server *server = zn_server_get_singleton();
  if (server->display_system != ZN_DISPLAY_SYSTEM_SCREEN) return;

  self->implementation->damage_whole(self->user_data);
}

void
zn_screen_send_frame_done(struct zn_screen *self, struct timespec *when)
{
  if (self->current_board) zn_board_send_frame_done(self->current_board, when);
}

void
zn_screen_get_screen_layout_coords(
    struct zn_screen *self, double x, double y, double *dest_x, double *dest_y)
{
  *dest_x = self->x + x;
  *dest_y = self->y + y;
}

void
zn_screen_get_effective_size(
    struct zn_screen *self, double *width, double *height)
{
  self->implementation->get_effective_size(self->user_data, width, height);
}

void
zn_screen_set_current_board(struct zn_screen *self, struct zn_board *board)
{
  struct zn_server *server = zn_server_get_singleton();

  if (self->current_board == board) {
    return;
  }

  if (self->current_board) {
    wl_list_remove(&self->current_board_destroy_listener.link);
    wl_list_init(&self->current_board_destroy_listener.link);
  }

  if (board) {
    if (!wl_list_empty(&board->view_list)) {
      struct zn_view *view =
          zn_container_of(board->view_list.prev, view, board_link);
      zn_scene_set_focused_view(server->scene, view);
    }

    wl_signal_add(
        &board->events.destroy, &self->current_board_destroy_listener);
  }

  self->current_board = board;

  struct zn_cursor *cursor = server->scene->cursor;
  if (self == board->screen) {
    cursor->grab->impl->rebase(cursor->grab);
  }

  wl_signal_emit(&self->events.current_board_changed, board);

  zn_screen_damage_whole(self);
}

void
zn_screen_switch_to_next_board(struct zn_screen *self)
{
  bool found = false;
  struct zn_board *board, *next_board = NULL;
  wl_list_for_each (board, &self->board_list, screen_link) {
    if (next_board == NULL) {
      next_board = board;
    }

    if (found) {
      next_board = board;
      break;
    }

    if (board == self->current_board) {
      found = true;
    }
  }

  if (!found) {
    return;
  }

  zn_screen_set_current_board(self, next_board);
}

void
zn_screen_switch_to_prev_board(struct zn_screen *self)
{
  bool found = false;
  struct zn_board *board, *next_board = NULL;
  wl_list_for_each_reverse (board, &self->board_list, screen_link) {
    if (next_board == NULL) {
      next_board = board;
    }

    if (found) {
      next_board = board;
      break;
    }

    if (board == self->current_board) {
      found = true;
    }
  }

  if (!found) {
    return;
  }

  zn_screen_set_current_board(self, next_board);
}

struct zn_screen *
zn_screen_create(
    const struct zn_screen_interface *implementation, void *user_data)
{
  struct zn_screen *self;
  struct zn_server *server = zn_server_get_singleton();

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->user_data = user_data;
  self->implementation = implementation;
  wl_list_init(&self->link);
  wl_list_init(&self->board_list);
  self->current_board = NULL;

  wl_signal_init(&self->events.current_board_changed);
  wl_signal_init(&self->events.destroy);

  self->current_board_destroy_listener.notify =
      zn_screen_handle_current_board_destroy;
  wl_list_init(&self->current_board_destroy_listener.link);

  self->zn_zigzag_layout = zn_zigzag_layout_create(self, server->renderer);
  if (self->zn_zigzag_layout == NULL) {
    zn_error("Failed to create the zn_zigzag_layout");
    goto err_screen;
  }

  return self;

err_screen:
  free(self);
err:
  return NULL;
}

void
zn_screen_destroy(struct zn_screen *self)
{
  struct zn_server *server = zn_server_get_singleton();
  zn_screen_layout_remove(server->scene->screen_layout, self);
  wl_signal_emit(&self->events.destroy, NULL);

  zn_zigzag_layout_destroy(self->zn_zigzag_layout);
  wl_list_remove(&self->board_list);
  wl_list_remove(&self->current_board_destroy_listener.link);
  wl_list_remove(&self->events.current_board_changed.listener_list);
  wl_list_remove(&self->events.destroy.listener_list);
  free(self);
}
