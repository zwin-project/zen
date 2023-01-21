#include "zen/screen/cursor-grab/move.h"

#include "zen-common.h"
#include "zen/appearance/cursor.h"
#include "zen/board.h"
#include "zen/cursor.h"
#include "zen/server.h"
#include "zen/view.h"

static void zn_move_cursor_grab_destroy(struct zn_move_cursor_grab *self);

static void
zn_move_cursor_grab_unset_maximized(struct zn_move_cursor_grab *self)
{
  if (self->view->maximize_status.maximized) {
    struct wlr_fbox view_fbox;
    zn_view_get_view_fbox(self->view, &view_fbox);
    // calculate relative pos
    self->view->maximize_status.reset_box.x =
        self->base.cursor->x - (self->view->maximize_status.reset_box.width *
                                   (self->base.cursor->x / view_fbox.width));
    self->view->maximize_status.reset_box.y =
        self->base.cursor->y + self->diff_y;
    self->diff_x =
        self->view->maximize_status.reset_box.x - self->base.cursor->x;
    self->diff_y =
        self->view->maximize_status.reset_box.y - self->base.cursor->y;
    zn_view_set_maximized(self->view, false);
  }
}

static void
zn_move_cursor_grab_motion_relative(
    struct zn_cursor_grab *grab, double dx, double dy, uint32_t time_msec)
{
  UNUSED(time_msec);
  struct zn_move_cursor_grab *self = zn_container_of(grab, self, base);

  zn_move_cursor_grab_unset_maximized(self);

  zn_cursor_move_relative(grab->cursor, dx, dy);
  zn_cursor_commit_appearance(grab->cursor);

  zn_view_move(self->view, grab->cursor->board, grab->cursor->x + self->diff_x,
      grab->cursor->y + self->diff_y);

  zn_view_commit_appearance(self->view);
}

static void
zn_move_cursor_grab_motion_absolute(struct zn_cursor_grab *grab,
    struct zn_board *board, double x, double y, uint32_t time_msec)
{
  UNUSED(time_msec);
  struct zn_move_cursor_grab *self = zn_container_of(grab, self, base);

  zn_move_cursor_grab_unset_maximized(self);

  zn_cursor_move(grab->cursor, board, x, y);
  zn_cursor_commit_appearance(grab->cursor);

  zn_view_move(self->view, grab->cursor->board, grab->cursor->x + self->diff_x,
      grab->cursor->y + self->diff_y);

  zn_view_commit_appearance(self->view);
}

static void
zn_move_cursor_grab_button(struct zn_cursor_grab *grab, uint32_t time_msec,
    uint32_t button, enum wlr_button_state state)
{
  UNUSED(time_msec);
  UNUSED(button);
  struct zn_move_cursor_grab *self = zn_container_of(grab, self, base);

  if (state == WLR_BUTTON_RELEASED) {
    zn_cursor_end_grab(grab->cursor);
  }
}

static void
zn_move_cursor_grab_axis(struct zn_cursor_grab *grab, uint32_t time_msec,
    enum wlr_axis_source source, enum wlr_axis_orientation orientation,
    double delta, int32_t delta_discrete)
{
  UNUSED(grab);
  UNUSED(time_msec);
  UNUSED(source);
  UNUSED(orientation);
  UNUSED(delta);
  UNUSED(delta_discrete);
}

static void
zn_move_cursor_grab_frame(struct zn_cursor_grab *grab)
{
  UNUSED(grab);
}

void
zn_move_cursor_grab_enter(
    struct zn_cursor_grab *grab, struct zn_board *board, double x, double y)
{
  zn_cursor_move(grab->cursor, board, x, y);

  zn_cursor_commit_appearance(grab->cursor);
}

void
zn_move_cursor_grab_leave(struct zn_cursor_grab *grab)
{
  zn_cursor_move(grab->cursor, NULL, 0, 0);

  zn_cursor_commit_appearance(grab->cursor);
}

static void
zn_move_cursor_grab_rebase(struct zn_cursor_grab *grab)
{
  struct zn_move_cursor_grab *self = zn_container_of(grab, self, base);
  zn_view_move(self->view, grab->cursor->board, grab->cursor->x + self->diff_x,
      grab->cursor->y + self->diff_y);
}

static void
zn_move_cursor_grab_cancel(struct zn_cursor_grab *grab)
{
  struct zn_move_cursor_grab *self = zn_container_of(grab, self, base);
  zn_cursor_set_xcursor(self->base.cursor, "left_ptr");
  zn_cursor_commit_appearance(grab->cursor);
  zn_move_cursor_grab_destroy(self);
}

static const struct zn_cursor_grab_interface implementation = {
    .motion_relative = zn_move_cursor_grab_motion_relative,
    .motion_absolute = zn_move_cursor_grab_motion_absolute,
    .button = zn_move_cursor_grab_button,
    .axis = zn_move_cursor_grab_axis,
    .frame = zn_move_cursor_grab_frame,
    .enter = zn_move_cursor_grab_enter,
    .leave = zn_move_cursor_grab_leave,
    .rebase = zn_move_cursor_grab_rebase,
    .cancel = zn_move_cursor_grab_cancel,
};

static void
zn_move_cursor_grab_handle_view_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_move_cursor_grab *self =
      zn_container_of(listener, self, view_destroy_listener);
  zn_cursor_end_grab(self->base.cursor);
}

static void
zn_move_cursor_grab_handle_init_board_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_move_cursor_grab *self =
      zn_container_of(listener, self, init_board_destroy_listener);
  zn_cursor_end_grab(self->base.cursor);
}

static struct zn_move_cursor_grab *
zn_move_cursor_grab_create(struct zn_cursor *cursor, struct zn_view *view)
{
  struct zn_move_cursor_grab *self;
  self = zalloc(sizeof *self);

  if (self == NULL) {
    zn_error("Failed to allocate memory");
    return NULL;
  }

  struct wlr_fbox view_box;
  zn_view_get_view_fbox(view, &view_box);
  self->init_board = view->board;
  self->init_x = view_box.x;
  self->init_y = view_box.y;
  self->diff_x = view_box.x - cursor->x;
  self->diff_y = view_box.y - cursor->y;
  self->view = view;
  self->base.impl = &implementation;

  self->view_destroy_listener.notify = zn_move_cursor_grab_handle_view_destroy;
  wl_signal_add(&view->events.destroy, &self->view_destroy_listener);

  self->init_board_destroy_listener.notify =
      zn_move_cursor_grab_handle_init_board_destroy;
  wl_signal_add(
      &view->board->events.destroy, &self->init_board_destroy_listener);

  return self;
}

static void
zn_move_cursor_grab_destroy(struct zn_move_cursor_grab *self)
{
  wl_list_remove(&self->view_destroy_listener.link);
  wl_list_remove(&self->init_board_destroy_listener.link);
  free(self);
}

void
zn_move_cursor_grab_start(struct zn_cursor *cursor, struct zn_view *view)
{
  struct zn_server *server = zn_server_get_singleton();
  struct wlr_seat *seat = server->input_manager->seat->wlr_seat;
  struct zn_move_cursor_grab *self = zn_move_cursor_grab_create(cursor, view);
  if (!self) {
    return;
  }

  view->impl->close_popups(view);
  zn_cursor_set_xcursor(cursor, "grabbing");
  zn_cursor_commit_appearance(cursor);
  wlr_seat_pointer_clear_focus(seat);
  zn_cursor_start_grab(cursor, &self->base);
}
