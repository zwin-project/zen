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

static const struct zn_cursor_grab_interface move_grab_interface = {
    .motion = move_grab_motion,
    .button = move_grab_button,
    .axis = move_grab_axis,
    .frame = move_grab_frame,
};

void
zn_cursor_grab_move_end(struct zn_cursor_grab_move* self)
{
  zn_cursor_set_xcursor(self->base.cursor, "left_ptr");
  zn_cursor_end_grab(self->base.cursor);
  free(self);
}

void
zn_cursor_grab_move_start(struct zn_cursor* cursor, struct zn_view* view)
{
  struct wlr_fbox cursor_box, view_box;
  struct zn_cursor_grab_move* self;
  self = zalloc(sizeof *self);

  if (self == NULL) {
    zn_error("Failed to allocate memory");
    return;
  }

  zn_cursor_get_fbox(cursor, &cursor_box);
  zn_view_get_surface_fbox(view, &view_box);
  self->diff_x = view_box.x - cursor_box.x;
  self->diff_y = view_box.y - cursor_box.y;
  self->view = view;
  self->prev_screen = cursor->screen;
  self->base.interface = &move_grab_interface;
  self->base.cursor = cursor;

  zn_cursor_set_xcursor(cursor, "grabbing");
  zn_cursor_start_grab(cursor, &self->base);
}
