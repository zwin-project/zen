#include "zen/input/cursor-grab-resize.h"

#include "zen-common.h"

static void zn_cursor_grab_resize_end(struct zn_cursor_grab_resize* self);

static void
resize_grab_motion(
    struct zn_cursor_grab* grab, struct wlr_event_pointer_motion* event)
{
  UNUSED(event);
  struct zn_cursor_grab_resize* self = zn_container_of(grab, self, base);
}

static void
resize_grab_button(
    struct zn_cursor_grab* grab, struct wlr_event_pointer_button* event)
{
  struct zn_cursor_grab_resize* self = zn_container_of(grab, self, base);

  if (event->state == WLR_BUTTON_RELEASED) {
    zn_cursor_grab_resize_end(self);
  }
}

static void
resize_grab_axis(
    struct zn_cursor_grab* grab, struct wlr_event_pointer_axis* event)
{
  UNUSED(grab);
  UNUSED(event);
}

static void
resize_grab_frame(struct zn_cursor_grab* grab)
{
  UNUSED(grab);
}

static void
resize_grab_rebase(struct zn_cursor_grab* grab)
{
  UNUSED(grab);
}

static void
resize_grab_cancel(struct zn_cursor_grab* grab)
{
  struct zn_cursor_grab_resize* self = zn_container_of(grab, self, base);
  zn_view_configure_by_fbox(self->view, &self->init_box);
  zn_cursor_grab_resize_end(self);
}

static const struct zn_cursor_grab_interface resize_grab_interface = {
    .motion = resize_grab_motion,
    .button = resize_grab_button,
    .axis = resize_grab_axis,
    .frame = resize_grab_frame,
    .rebase = resize_grab_rebase,
    .cancel = resize_grab_cancel,
};

static void
zn_cursor_grab_resize_handle_view_unmap(
    struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zn_cursor_grab_resize* self =
      zn_container_of(listener, self, view_unmap_listener);
  zn_cursor_grab_resize_end(self);
}

static struct zn_cursor_grab_resize*
zn_cursor_grab_resize_create(
    struct zn_cursor* cursor, struct zn_view* view, uint32_t edges)
{
  struct zn_cursor_grab_resize* self;
  self = zalloc(sizeof *self);

  if (self == NULL) {
    zn_error("Failed to allocate memory");
    return NULL;
  }

  zn_view_get_window_fbox(view, &self->init_box);

  self->view = view;
  self->edges = edges;
  self->base.interface = &resize_grab_interface;
  self->base.cursor = cursor;

  self->view_unmap_listener.notify = zn_cursor_grab_resize_handle_view_unmap;
  wl_signal_add(&view->events.unmap, &self->view_unmap_listener);

  return self;
}

static void
zn_cursor_grab_resize_destroy(struct zn_cursor_grab_resize* self)
{
  wl_list_remove(&self->view_unmap_listener.link);
  free(self);
}

static void
zn_cursor_grab_resize_end(struct zn_cursor_grab_resize* self)
{
  zn_cursor_end_grab(self->base.cursor);
  zn_cursor_grab_resize_destroy(self);
}

void
zn_cursor_grab_resize_start(
    struct zn_cursor* cursor, struct zn_view* view, uint32_t edges)
{
  struct zn_server* server = zn_server_get_singleton();
  struct wlr_seat* seat = server->input_manager->seat->wlr_seat;

  struct zn_cursor_grab_resize* self =
      zn_cursor_grab_resize_create(cursor, view, edges);
  if (!self) {
    return;
  }

  wlr_seat_pointer_clear_focus(seat);
  zn_cursor_start_grab(cursor, &self->base);
}
