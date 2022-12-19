#include "zen/screen/cursor-grab/down.h"

#include <wlr/types/wlr_seat.h>
#include <zen-common.h>

#include "zen/server.h"
#include "zen/view.h"

static void zn_down_cursor_grab_destroy(struct zn_down_cursor_grab *self);

static void
zn_down_cursor_grab_send_movement(
    struct zn_down_cursor_grab *self, uint32_t time_msec)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_cursor *cursor = self->base.cursor;
  struct zn_seat *seat = server->input_manager->seat;

  if (!self->view->board || self->view->board != cursor->board) return;

  struct wlr_fbox view_surface_fbox;
  double view_sx, view_sy;
  zn_view_get_surface_fbox(self->view, &view_surface_fbox);

  view_sx = cursor->x - view_surface_fbox.x;
  view_sy = cursor->y - view_surface_fbox.y;

  double sx, sy;
  struct wlr_surface *surface = self->view->impl->get_wlr_surface_at(
      self->view, view_sx, view_sy, &sx, &sy);

  if (!surface) {
    surface = self->view->surface;
    sx = view_sx;
    sy = view_sy;
  }

  wlr_seat_pointer_enter(seat->wlr_seat, surface, sx, sy);
  wlr_seat_pointer_send_motion(seat->wlr_seat, time_msec, sx, sy);
}

static void
zn_down_cursor_grab_motion_relative(
    struct zn_cursor_grab *grab, double dx, double dy, uint32_t time_msec)
{
  struct zn_down_cursor_grab *self = zn_container_of(grab, self, base);

  zn_cursor_move_relative(grab->cursor, dx, dy);

  zn_down_cursor_grab_send_movement(self, time_msec);
}

static void
zn_down_cursor_grab_motion_absolute(struct zn_cursor_grab *grab,
    struct zn_board *board, double x, double y, uint32_t time_msec)
{
  struct zn_down_cursor_grab *self = zn_container_of(grab, self, base);

  zn_cursor_move(grab->cursor, board, x, y);

  zn_down_cursor_grab_send_movement(self, time_msec);
}

static void
zn_down_cursor_grab_button(struct zn_cursor_grab *grab, uint32_t time_msec,
    uint32_t button, enum wlr_button_state state)
{
  UNUSED(grab);
  struct zn_server *server = zn_server_get_singleton();
  struct zn_seat *seat = server->input_manager->seat;

  wlr_seat_pointer_send_button(seat->wlr_seat, time_msec, button, state);

  if (seat->pressing_button_count == 0 && state == WLR_BUTTON_RELEASED) {
    zn_cursor_end_grab(grab->cursor);
  }
}

static void
zn_down_cursor_grab_axis(struct zn_cursor_grab *grab, uint32_t time_msec,
    enum wlr_axis_source source, enum wlr_axis_orientation orientation,
    double delta, int32_t delta_discrete)
{
  UNUSED(grab);
  struct zn_server *server = zn_server_get_singleton();
  struct wlr_seat *seat = server->input_manager->seat->wlr_seat;

  wlr_seat_pointer_send_axis(
      seat, time_msec, orientation, delta, delta_discrete, source);
}

static void
zn_down_cursor_grab_frame(struct zn_cursor_grab *grab)
{
  UNUSED(grab);
  struct zn_server *server = zn_server_get_singleton();
  struct wlr_seat *seat = server->input_manager->seat->wlr_seat;

  wlr_seat_pointer_send_frame(seat);
}

void
zn_down_cursor_grab_enter(
    struct zn_cursor_grab *grab, struct zn_board *board, double x, double y)
{
  UNUSED(grab);
  UNUSED(board);
  UNUSED(x);
  UNUSED(y);
}

void
zn_down_cursor_grab_leave(struct zn_cursor_grab *grab)
{
  UNUSED(grab);
}

static void
zn_down_cursor_grab_rebase(struct zn_cursor_grab *grab)
{
  UNUSED(grab);
}

static void
zn_down_cursor_grab_cancel(struct zn_cursor_grab *grab)
{
  struct zn_down_cursor_grab *self = zn_container_of(grab, self, base);
  zn_down_cursor_grab_destroy(self);
}

static const struct zn_cursor_grab_interface implementation = {
    .motion_relative = zn_down_cursor_grab_motion_relative,
    .motion_absolute = zn_down_cursor_grab_motion_absolute,
    .button = zn_down_cursor_grab_button,
    .axis = zn_down_cursor_grab_axis,
    .frame = zn_down_cursor_grab_frame,
    .enter = zn_down_cursor_grab_enter,
    .leave = zn_down_cursor_grab_leave,
    .rebase = zn_down_cursor_grab_rebase,
    .cancel = zn_down_cursor_grab_cancel,
};

static void
zn_down_cursor_grab_handle_view_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_down_cursor_grab *self =
      zn_container_of(listener, self, view_destroy_listener);

  zn_cursor_end_grab(self->base.cursor);
}

static struct zn_down_cursor_grab *
zn_down_cursor_grab_create(struct zn_view *view)
{
  struct zn_down_cursor_grab *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    goto err;
  }

  self->base.impl = &implementation;
  self->view = view;

  self->view_destroy_listener.notify = zn_down_cursor_grab_handle_view_destroy;
  wl_signal_add(&view->events.destroy, &self->view_destroy_listener);

  return self;

err:
  return NULL;
}

static void
zn_down_cursor_grab_destroy(struct zn_down_cursor_grab *self)
{
  wl_list_remove(&self->view_destroy_listener.link);
  free(self);
}

void
zn_down_cursor_grab_start(struct zn_cursor *cursor, struct zn_view *view)
{
  struct zn_down_cursor_grab *self = zn_down_cursor_grab_create(view);
  if (!self) {
    zn_error("Failed to create button cursor grab");
    return;
  }

  zn_cursor_start_grab(cursor, &self->base);
}
