#include "zen/input/cursor-grab-move.h"

#include "zen-common.h"

void zn_cursor_grab_move_end(struct zn_cursor_grab_move* self);

static void
move_grab_motion(
    struct zn_cursor_grab* grab, struct wlr_event_pointer_motion* event)
{
  UNUSED(event);
  struct zn_cursor_grab_move* self = zn_container_of(grab, self, base);

  if (grab->cursor->screen != self->prev_screen) {
    zn_view_change_board(self->view, grab->cursor->screen->current_board);
  }

  zn_view_move(self->view, grab->cursor->x + self->diff_x,
      grab->cursor->y + self->diff_y);
  self->prev_screen = grab->cursor->screen;
}

static void
move_grab_button(
    struct zn_cursor_grab* grab, struct wlr_event_pointer_button* event)
{
  struct zn_cursor_grab_move* self = zn_container_of(grab, self, base);

  if (event->state == WLR_BUTTON_RELEASED) {
    zn_cursor_grab_move_end(self);
  }
}

static void
move_grab_axis(
    struct zn_cursor_grab* grab, struct wlr_event_pointer_axis* event)
{
  UNUSED(grab);
  UNUSED(event);
}

static void
move_grab_frame(struct zn_cursor_grab* grab)
{
  UNUSED(grab);
}

static void
move_grab_cancel(struct zn_cursor_grab* grab)
{
  struct zn_cursor_grab_move* self = zn_container_of(grab, self, base);
  zn_view_move(self->view, self->init_x, self->init_y);
  zn_cursor_grab_move_end(self);
}

static const struct zn_cursor_grab_interface move_grab_interface = {
    .motion = move_grab_motion,
    .button = move_grab_button,
    .axis = move_grab_axis,
    .frame = move_grab_frame,
    .cancel = move_grab_cancel,
};

static void
zn_cursor_grab_move_handle_view_unmap(struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zn_cursor_grab_move* self =
      zn_container_of(listener, self, view_unmap_listener);
  zn_cursor_grab_move_end(self);
}

void
zn_cursor_grab_move_end(struct zn_cursor_grab_move* self)
{
  wl_list_remove(&self->view_unmap_listener.link);
  zn_cursor_set_xcursor(self->base.cursor, "left_ptr");
  zn_cursor_end_grab(self->base.cursor);
  free(self);
}

void
zn_cursor_grab_move_start(struct zn_cursor* cursor, struct zn_view* view)
{
  struct zn_server* server = zn_server_get_singleton();
  struct wlr_seat* seat = server->input_manager->seat->wlr_seat;
  struct wlr_fbox view_box;
  struct zn_cursor_grab_move* self;
  self = zalloc(sizeof *self);

  if (self == NULL) {
    zn_error("Failed to allocate memory");
    return;
  }

  zn_view_get_surface_fbox(view, &view_box);
  self->init_x = view_box.x;
  self->init_y = view_box.y;
  self->diff_x = view_box.x - cursor->x;
  self->diff_y = view_box.y - cursor->y;
  self->view = view;
  self->base.interface = &move_grab_interface;
  self->base.cursor = cursor;

  self->view_unmap_listener.notify = zn_cursor_grab_move_handle_view_unmap;
  wl_signal_add(&view->events.unmap, &self->view_unmap_listener);

  zn_cursor_set_xcursor(cursor, "grabbing");
  wlr_seat_pointer_notify_clear_focus(seat);
  zn_cursor_start_grab(cursor, &self->base);
}
