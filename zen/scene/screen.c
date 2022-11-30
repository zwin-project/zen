#include "zen/scene/screen.h"

#include "zen-common.h"
#include "zen/input/seat.h"
#include "zen/scene/board.h"
#include "zen/scene/screen-layout.h"
#include "zen/scene/view.h"
#include "zen/wlr/box.h"

struct surface_callback_data {
  zn_screen_for_each_visible_surface_callback_t callback;
  void *user_data;
};

static void
zn_screen_for_each_visible_surface_callback(
    struct wlr_surface *surface, int sx, int sy, void *_data)
{
  UNUSED(sx);
  UNUSED(sy);
  struct surface_callback_data *data = _data;

  data->callback(surface, data->user_data);
}

void
zn_screen_for_each_visible_surface(struct zn_screen *self,
    zn_screen_for_each_visible_surface_callback_t callback, void *user_data)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_cursor *cursor = server->input_manager->seat->cursor;
  struct zn_board *board = zn_screen_get_current_board(self);
  struct zn_view *view;
  struct surface_callback_data data = {
      .callback = callback,
      .user_data = user_data,
  };

  wl_list_for_each (view, &board->view_list, link) {
    callback(view->impl->get_wlr_surface(view), user_data);

    if (view->impl->for_each_popup_surface)
      view->impl->for_each_popup_surface(
          view, zn_screen_for_each_visible_surface_callback, &data);
  }

  if (cursor->screen == self && cursor->surface != NULL) {
    callback(cursor->surface, user_data);
  }
}

struct wlr_surface *
zn_screen_get_surface_at(struct zn_screen *self, double x, double y,
    double *surface_x, double *surface_y, struct zn_view **view)
{
  struct zn_view *view_iterator;
  struct zn_board *board = zn_screen_get_current_board(self);
  double view_sx, view_sy;
  struct wlr_surface *surface;

  wl_list_for_each_reverse (view_iterator, &board->view_list, link) {
    struct wlr_fbox fbox;
    zn_view_get_surface_fbox(view_iterator, &fbox);
    view_sx = x - fbox.x;
    view_sy = y - fbox.y;

    double sx, sy;
    surface = view_iterator->impl->get_wlr_surface_at(
        view_iterator, view_sx, view_sy, &sx, &sy);
    if (surface != NULL) {
      if (surface_x) *surface_x = sx;
      if (surface_y) *surface_y = sy;
      if (view) *view = view_iterator;
      return surface;
    }
  }

  return NULL;
}

void
zn_screen_switch_to_next_board(struct zn_screen *self)
{
  struct zn_board *board, *next_board = NULL,
                          *current_board = zn_screen_get_current_board(self);
  bool found = false;

  wl_list_for_each (board, &self->board_list, screen_link) {
    if (next_board == NULL) {
      next_board = board;
    }

    if (found) {
      next_board = board;
      break;
    }

    if (board == current_board) {
      found = true;
    }
  }

  if (!zn_assert(found,
          "Couldn't find zn_screen's current board in zn_screen::board_list")) {
    return;
  }

  zn_screen_set_current_board(self, next_board);
}

void
zn_screen_switch_to_prev_board(struct zn_screen *self)
{
  struct zn_board *board, *next_board = NULL,
                          *current_board = zn_screen_get_current_board(self);
  bool found = false;

  wl_list_for_each_reverse (board, &self->board_list, screen_link) {
    if (next_board == NULL) {
      next_board = board;
    }

    if (found) {
      next_board = board;
      break;
    }

    if (board == current_board) {
      found = true;
    }
  }

  if (!zn_assert(found,
          "Couldn't find zn_screen's current board in zn_screen::board_list")) {
    return;
  }

  zn_screen_set_current_board(self, next_board);
}

static void
zn_screen_handle_current_board_screen_assigned(
    struct wl_listener *listener, void *data)
{
  struct zn_screen *self =
      zn_container_of(listener, self, current_board_screen_assigned_listener);
  struct zn_board_screen_assigned_event *event = data;
  struct zn_board *board = NULL;

  if (event->current_screen == self) {
    return;
  }

  if (!wl_list_empty(&self->board_list)) {
    board = zn_container_of(self->board_list.next, board, screen_link);
  }

  /* board can be NULL here only when destroying this screen */
  zn_screen_set_current_board(self, board);
}

/** check if self has given board in its board_list */
static bool
zn_screen_has_board(struct zn_screen *self, struct zn_board *target_board)
{
  struct zn_board *board;
  wl_list_for_each (board, &self->board_list, screen_link) {
    if (board == target_board) {
      return true;
    }
  }
  return false;
}

void
zn_screen_set_current_board(struct zn_screen *self, struct zn_board *board)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_cursor *cursor = server->input_manager->seat->cursor;
  struct zn_view *view;

  if (self->current_board == board) {
    return;
  }

  if (self->current_board) {
    wl_list_remove(&self->current_board_screen_assigned_listener.link);
    wl_list_init(&self->current_board_screen_assigned_listener.link);
  }

  if (board) {
    if (!zn_assert(zn_screen_has_board(self, board),
            "Tried to set current board but the board doesn't belong to the "
            "screen.")) {
      return;
    }
    wl_signal_add(&board->events.screen_assigned,
        &self->current_board_screen_assigned_listener);

    if (!wl_list_empty(&board->view_list)) {
      view = zn_container_of(board->view_list.prev, view, link);
      zn_scene_set_focused_view(server->scene, view);
    }
  }

  self->current_board = board;

  if (self == cursor->screen) {
    cursor->grab->interface->rebase(cursor->grab);
  }

  zn_output_add_damage_whole(self->output);
}

struct zn_board *
zn_screen_get_current_board(struct zn_screen *self)
{
  return self->current_board;
}

void
zn_screen_get_screen_layout_coords(
    struct zn_screen *self, double x, double y, double *dst_x, double *dst_y)
{
  *dst_x = self->x + x;
  *dst_y = self->y + y;
}

void
zn_screen_get_fbox(struct zn_screen *self, struct wlr_fbox *box)
{
  int width, height;
  wlr_output_effective_resolution(self->output->wlr_output, &width, &height);
  box->x = self->x;
  box->y = self->y;
  box->width = width;
  box->height = height;
}

struct zn_screen *
zn_screen_create(
    struct zn_screen_layout *screen_layout, struct zn_output *output)
{
  struct zn_screen *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->output = output;
  self->screen_layout = screen_layout;
  wl_signal_init(&self->events.destroy);
  wl_list_init(&self->board_list);
  self->current_board = NULL;
  self->current_board_screen_assigned_listener.notify =
      zn_screen_handle_current_board_screen_assigned;
  wl_list_init(&self->current_board_screen_assigned_listener.link);

  zn_screen_layout_add(screen_layout, self);

  return self;

err:
  return NULL;
}

void
zn_screen_destroy(struct zn_screen *self)
{
  wl_list_remove(&self->current_board_screen_assigned_listener.link);
  wl_signal_emit(&self->events.destroy, NULL);

  zn_screen_layout_remove(self->screen_layout, self);
  free(self);
}
