#include "zen/screen/cursor-grab/resize.h"

#include "zen-common.h"
#include "zen/appearance/cursor.h"
#include "zen/board.h"
#include "zen/cursor.h"
#include "zen/server.h"
#include "zen/view.h"

static void zn_resize_cursor_grab_destroy(struct zn_resize_cursor_grab *self);

static void
zn_resize_cursor_grab_motion_relative(
    struct zn_cursor_grab *grab, double dx, double dy, uint32_t time_msec)
{
  UNUSED(time_msec);
  struct zn_resize_cursor_grab *self = zn_container_of(grab, self, base);

  zn_cursor_move_relative(grab->cursor, dx, dy);
  zna_cursor_commit(grab->cursor->appearance, ZNA_CURSOR_DAMAGE_GEOMETRY);

  if (self->view->board != grab->cursor->board) {
    return;
  }

  double width = self->init_view_width;
  double height = self->init_view_height;
  const double diff_width = grab->cursor->x - self->init_cursor_x;
  const double diff_height = grab->cursor->y - self->init_cursor_y;

  if (self->view->resize_status.edges & WLR_EDGE_LEFT) {
    width -= diff_width;
  }
  if (self->view->resize_status.edges & WLR_EDGE_RIGHT) {
    width += diff_width;
  }
  if (self->view->resize_status.edges & WLR_EDGE_TOP) {
    height -= diff_height;
  }
  if (self->view->resize_status.edges & WLR_EDGE_BOTTOM) {
    height += diff_height;
  }

  self->view->resize_status.resizing = true;
  self->view->resize_status.last_serial =
      self->view->impl->set_size(self->view, width, height);
}

static void
zn_resize_cursor_grab_motion_absolute(struct zn_cursor_grab *grab,
    struct zn_board *board, double x, double y, uint32_t time_msec)
{
  UNUSED(time_msec);

  zn_cursor_move(grab->cursor, board, x, y);

  zna_cursor_commit(grab->cursor->appearance, ZNA_CURSOR_DAMAGE_GEOMETRY);
}

static void
zn_resize_cursor_grab_button(struct zn_cursor_grab *grab, uint32_t time_msec,
    uint32_t button, enum wlr_button_state state)
{
  UNUSED(time_msec);
  UNUSED(button);
  struct zn_resize_cursor_grab *self = zn_container_of(grab, self, base);

  if (state == WLR_BUTTON_RELEASED) {
    zn_cursor_end_grab(grab->cursor);
  }
}

static void
zn_resize_cursor_grab_axis(struct zn_cursor_grab *grab, uint32_t time_msec,
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
zn_resize_cursor_grab_frame(struct zn_cursor_grab *grab)
{
  UNUSED(grab);
}

void
zn_resize_cursor_grab_enter(
    struct zn_cursor_grab *grab, struct zn_board *board, double x, double y)
{
  zn_cursor_move(grab->cursor, board, x, y);

  zna_cursor_commit(grab->cursor->appearance, ZNA_CURSOR_DAMAGE_GEOMETRY);
}

void
zn_resize_cursor_grab_leave(struct zn_cursor_grab *grab)
{
  zn_cursor_move(grab->cursor, NULL, 0, 0);

  zna_cursor_commit(grab->cursor->appearance, ZNA_CURSOR_DAMAGE_GEOMETRY);
}

static void
zn_resize_cursor_grab_rebase(struct zn_cursor_grab *grab)
{
  UNUSED(grab);
}

static void
zn_resize_cursor_grab_cancel(struct zn_cursor_grab *grab)
{
  struct zn_resize_cursor_grab *self = zn_container_of(grab, self, base);

  zn_cursor_set_xcursor(self->base.cursor, "left_ptr");
  zn_resize_cursor_grab_destroy(self);
}

static const struct zn_cursor_grab_interface implementation = {
    .motion_relative = zn_resize_cursor_grab_motion_relative,
    .motion_absolute = zn_resize_cursor_grab_motion_absolute,
    .button = zn_resize_cursor_grab_button,
    .axis = zn_resize_cursor_grab_axis,
    .frame = zn_resize_cursor_grab_frame,
    .enter = zn_resize_cursor_grab_enter,
    .leave = zn_resize_cursor_grab_leave,
    .rebase = zn_resize_cursor_grab_rebase,
    .cancel = zn_resize_cursor_grab_cancel,
};

static void
zn_resize_cursor_grab_handle_view_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_resize_cursor_grab *self =
      zn_container_of(listener, self, view_destroy_listener);
  zn_cursor_end_grab(self->base.cursor);
}

static struct zn_resize_cursor_grab *
zn_resize_cursor_grab_create(
    struct zn_cursor *cursor, struct zn_view *view, uint32_t edges)
{
  struct zn_resize_cursor_grab *self;
  self = zalloc(sizeof *self);

  if (self == NULL) {
    zn_error("Failed to allocate memory");
    return NULL;
  }

  struct wlr_fbox box;
  zn_view_get_view_fbox(view, &box);

  self->init_view_width = box.width;
  self->init_view_height = box.height;
  self->init_cursor_x = cursor->x;
  self->init_cursor_y = cursor->y;

  self->view = view;
  self->base.impl = &implementation;
  self->base.cursor = cursor;

  view->resize_status.edges = edges;

  self->view_destroy_listener.notify =
      zn_resize_cursor_grab_handle_view_destroy;
  wl_signal_add(&view->events.destroy, &self->view_destroy_listener);

  return self;
}

static void
zn_resize_cursor_grab_destroy(struct zn_resize_cursor_grab *self)
{
  wl_list_remove(&self->view_destroy_listener.link);
  free(self);
}

void
zn_resize_cursor_grab_start(
    struct zn_cursor *cursor, struct zn_view *view, uint32_t edges)
{
  if (view->maximize_status.maximized) {
    return;
  }

  struct zn_server *server = zn_server_get_singleton();
  struct wlr_seat *seat = server->input_manager->seat->wlr_seat;

  struct zn_resize_cursor_grab *self =
      zn_resize_cursor_grab_create(cursor, view, edges);
  if (!self) {
    return;
  }

  const char *xcursor_name[] = {
      [WLR_EDGE_TOP] = "top_corner",
      [WLR_EDGE_BOTTOM] = "bottom_corner",
      [WLR_EDGE_LEFT] = "left_corner",
      [WLR_EDGE_RIGHT] = "right_corner",
      [WLR_EDGE_TOP | WLR_EDGE_LEFT] = "top_left_corner",
      [WLR_EDGE_TOP | WLR_EDGE_RIGHT] = "top_right_corner",
      [WLR_EDGE_BOTTOM | WLR_EDGE_LEFT] = "bottom_left_corner",
      [WLR_EDGE_BOTTOM | WLR_EDGE_RIGHT] = "bottom_right_corner",
  };

  wlr_seat_pointer_clear_focus(seat);
  zn_cursor_set_xcursor(cursor, xcursor_name[edges]);
  zn_cursor_start_grab(cursor, &self->base);
}
