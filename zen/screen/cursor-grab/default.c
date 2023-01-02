#include "zen/screen/cursor-grab/default.h"

#include <time.h>
#include <zen-common.h>

#include "zen/appearance/cursor.h"
#include "zen/board.h"
#include "zen/cursor.h"
#include "zen/scene.h"
#include "zen/screen-layout.h"
#include "zen/screen.h"
#include "zen/screen/cursor-grab/down.h"
#include "zen/server.h"
#include "zen/ui/zigzag-layout.h"
#include "zen/view.h"

static void
zn_default_cursor_grab_send_movement(
    struct zn_cursor_grab *grab, uint32_t time_msec, bool no_motion)
{
  if (!grab->cursor->board) {
    return;
  }

  struct zn_server *server = zn_server_get_singleton();
  struct wlr_seat *seat = server->input_manager->seat->wlr_seat;

  double surface_x, surface_y;
  struct wlr_surface *surface = zn_board_get_surface_at(grab->cursor->board,
      grab->cursor->x, grab->cursor->y, &surface_x, &surface_y, NULL);

  if (surface) {
    wlr_seat_pointer_enter(seat, surface, surface_x, surface_y);
    if (!no_motion) {
      wlr_seat_pointer_send_motion(seat, time_msec, surface_x, surface_y);
    }
  } else {
    zn_cursor_set_xcursor(grab->cursor, "left_ptr");
    wlr_seat_pointer_clear_focus(seat);
  }
}

void
zn_default_cursor_grab_motion_relative(
    struct zn_cursor_grab *grab, double dx, double dy, uint32_t time_msec)
{
  if (!grab->cursor->board || !grab->cursor->board->screen) {
    return;
  }

  zn_cursor_move_relative(grab->cursor, dx, dy);

  zn_default_cursor_grab_send_movement(grab, time_msec, false);

  zn_cursor_commit_appearance(grab->cursor);
}

void
zn_default_cursor_grab_motion_absolute(struct zn_cursor_grab *grab,
    struct zn_board *board, double x, double y, uint32_t time_msec)
{
  zn_cursor_move(grab->cursor, board, x, y);

  zn_default_cursor_grab_send_movement(grab, time_msec, false);

  zn_cursor_commit_appearance(grab->cursor);
}

void
zn_default_cursor_grab_button(struct zn_cursor_grab *grab, uint32_t time_msec,
    uint32_t button, enum wlr_button_state state)
{
  UNUSED(grab);
  struct zn_server *server = zn_server_get_singleton();
  struct wlr_seat *seat = server->input_manager->seat->wlr_seat;
  struct zn_cursor *cursor = grab->cursor;

  if (!cursor->board) {
    return;
  }

  wlr_seat_pointer_send_button(seat, time_msec, button, state);

  if (state == WLR_BUTTON_PRESSED) {
    bool result = zn_zigzag_layout_notify_click(
        cursor->board->screen->zn_zigzag_layout, cursor->x, cursor->y);
    if (result) return;
    struct zn_view *view = NULL;
    zn_board_get_surface_at(
        cursor->board, cursor->x, cursor->y, NULL, NULL, &view);
    zn_scene_set_focused_view(server->scene, view);

    if (view) {
      struct zn_view *view_iter;
      wl_list_for_each (view_iter, &view->board->view_list, board_link) {
        zn_view_commit_appearance(view_iter);
      }
    }

    if (view && server->input_manager->seat->pressing_button_count == 1) {
      zn_down_cursor_grab_start(cursor, view);
    }
  }
}

void
zn_default_cursor_grab_axis(struct zn_cursor_grab *grab, uint32_t time_msec,
    enum wlr_axis_source source, enum wlr_axis_orientation orientation,
    double delta, int32_t delta_discrete)
{
  UNUSED(grab);
  struct zn_server *server = zn_server_get_singleton();
  struct wlr_seat *seat = server->input_manager->seat->wlr_seat;

  wlr_seat_pointer_send_axis(
      seat, time_msec, orientation, delta, delta_discrete, source);
}

void
zn_default_cursor_grab_frame(struct zn_cursor_grab *grab)
{
  UNUSED(grab);
  struct zn_server *server = zn_server_get_singleton();
  struct wlr_seat *seat = server->input_manager->seat->wlr_seat;

  wlr_seat_pointer_send_frame(seat);
}

void
zn_default_cursor_grab_enter(
    struct zn_cursor_grab *grab, struct zn_board *board, double x, double y)
{
  zn_cursor_move(grab->cursor, board, x, y);

  zn_cursor_commit_appearance(grab->cursor);
}

void
zn_default_cursor_grab_leave(struct zn_cursor_grab *grab)
{
  zn_cursor_move(grab->cursor, NULL, 0, 0);

  zn_cursor_commit_appearance(grab->cursor);
}

void
zn_default_cursor_grab_rebase(struct zn_cursor_grab *grab)
{
  zn_default_cursor_grab_send_movement(grab, 0, true);

  zn_cursor_commit_appearance(grab->cursor);
}

void
zn_default_cursor_grab_cancel(struct zn_cursor_grab *grab)
{
  UNUSED(grab);
}

static const struct zn_cursor_grab_interface implementation = {
    .motion_relative = zn_default_cursor_grab_motion_relative,
    .motion_absolute = zn_default_cursor_grab_motion_absolute,
    .button = zn_default_cursor_grab_button,
    .axis = zn_default_cursor_grab_axis,
    .frame = zn_default_cursor_grab_frame,
    .enter = zn_default_cursor_grab_enter,
    .leave = zn_default_cursor_grab_leave,
    .rebase = zn_default_cursor_grab_rebase,
    .cancel = zn_default_cursor_grab_cancel,
};

struct zn_default_cursor_grab *
zn_default_cursor_grab_create(void)
{
  struct zn_default_cursor_grab *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->base.impl = &implementation;

  return self;

err:
  return NULL;
}

void
zn_default_cursor_grab_destroy(struct zn_default_cursor_grab *self)
{
  free(self);
}
