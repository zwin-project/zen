#include "zen/screen/cursor-grab/default.h"

#include <zen-common.h>

#include "zen/appearance/cursor.h"
#include "zen/board.h"
#include "zen/cursor.h"
#include "zen/scene.h"
#include "zen/screen-layout.h"
#include "zen/screen.h"
#include "zen/server.h"

static void
zn_default_cursor_grab_send_motion(
    struct zn_cursor_grab *grab, uint32_t time_msec)
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
    wlr_seat_pointer_send_motion(seat, time_msec, surface_x, surface_y);
  } else {
    zn_cursor_set_xcursor(grab->cursor, "left_ptr");
    wlr_seat_pointer_clear_focus(seat);
  }
}

void
zn_default_cursor_grab_motion_relative(
    struct zn_cursor_grab *grab, double dx, double dy, uint32_t time_msec)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_cursor *cursor = grab->cursor;

  if (!cursor->board || !cursor->board->screen) {
    return;
  }

  double layout_x, layout_y;
  zn_screen_get_screen_layout_coords(cursor->board->screen, cursor->x + dx,
      cursor->y + dy, &layout_x, &layout_y);

  double screen_x, screen_y;
  struct zn_screen *screen = zn_screen_layout_get_closest_screen(
      server->scene->screen_layout, layout_x, layout_y, &screen_x, &screen_y);

  zn_cursor_move(cursor, screen->board, screen_x, screen_y);

  zn_default_cursor_grab_send_motion(grab, time_msec);

  zna_cursor_commit(cursor->appearance, ZNA_CURSOR_DAMAGE_GEOMETRY);
}

void
zn_default_cursor_grab_motion_absolute(struct zn_cursor_grab *grab,
    struct zn_board *board, double x, double y, uint32_t time_msec)
{
  zn_cursor_move(grab->cursor, board, x, y);

  zn_default_cursor_grab_send_motion(grab, time_msec);

  zna_cursor_commit(grab->cursor->appearance, ZNA_CURSOR_DAMAGE_GEOMETRY);

  UNUSED(time_msec);
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
    struct zn_view *view = NULL;
    zn_board_get_surface_at(
        cursor->board, cursor->x, cursor->y, NULL, NULL, &view);
    zn_scene_set_focused_view(server->scene, view);
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

  zna_cursor_commit(grab->cursor->appearance, ZNA_CURSOR_DAMAGE_GEOMETRY);
}

void
zn_default_cursor_grab_leave(struct zn_cursor_grab *grab)
{
  zn_cursor_move(grab->cursor, NULL, 0, 0);

  zna_cursor_commit(grab->cursor->appearance, ZNA_CURSOR_DAMAGE_GEOMETRY);
}

void
zn_default_cursor_grab_rebase(struct zn_cursor_grab *grab)
{
  UNUSED(grab);
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
