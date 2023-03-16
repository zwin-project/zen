#include "seat.h"

#include <cglm/vec2.h>

#include "cursor.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/snode.h"

#define DEFAULT_SEAT "seat0"

static void
zn_seat_handle_cursor_focus_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_seat *self =
      zn_container_of(listener, self, cursor_focus_destroy_listener);
  zn_seat_pointer_enter(self, NULL, GLM_VEC2_ZERO);
}

static void
zn_seat_handle_focus_destroy(struct wl_listener *listener, void *data UNUSED)
{
  struct zn_seat *self =
      zn_container_of(listener, self, focus_destroy_listener);

  zn_seat_set_focus(self, NULL);
}

struct zn_screen *
zn_seat_get_focused_screen(struct zn_seat *self)
{
  struct zn_screen *focus_screen = NULL;

  // TODO(@Aki-7): Respect keyboard focus

  if (self->pointer_state.focus) {
    focus_screen = self->pointer_state.focus->screen;
  }

  if (!focus_screen) {
    focus_screen = self->cursor->snode->screen;
  }

  return focus_screen;
}

void
zn_seat_set_capabilities(struct zn_seat *self, uint32_t capabilities)
{
  wlr_seat_set_capabilities(self->wlr_seat, capabilities);
}

void
zn_seat_set_focus(struct zn_seat *self, struct zn_snode *snode)
{
  if (!zn_assert(snode == NULL || zn_snode_is_focusable(snode),
          "zn_seat_set_focus: snode must be NULL or focusable")) {
    return;
  }

  if (self->focus == snode) {
    return;
  }

  if (self->focus) {
    wl_list_remove(&self->focus_destroy_listener.link);
    wl_list_init(&self->focus_destroy_listener.link);
    zn_snode_unfocus(self->focus);
  }

  self->focus = snode;

  if (snode) {
    wl_signal_add(&snode->events.destroy, &self->focus_destroy_listener);
    zn_snode_focus(snode);
  }
}

void
zn_seat_pointer_button(struct zn_seat *self, uint32_t time_msec,
    uint32_t button, enum wlr_button_state state)
{
  if (self->pointer_state.focus == NULL) {
    return;
  }

  zn_snode_pointer_button(self->pointer_state.focus, time_msec, button, state);
}

void
zn_seat_pointer_enter(struct zn_seat *self, struct zn_snode *snode, vec2 point)
{
  if (self->pointer_state.focus == snode) {
    return;
  }

  if (self->pointer_state.focus) {
    zn_snode_pointer_leave(self->pointer_state.focus);
    wl_list_remove(&self->cursor_focus_destroy_listener.link);
    wl_list_init(&self->cursor_focus_destroy_listener.link);
  }

  self->pointer_state.focus = snode;

  if (snode) {
    zn_snode_pointer_enter(snode, point);
    wl_signal_add(&snode->events.destroy, &self->cursor_focus_destroy_listener);
  }
}

void
zn_seat_pointer_motion(struct zn_seat *self, uint32_t time_msec, vec2 point)
{
  if (self->pointer_state.focus == NULL) {
    return;
  }

  zn_snode_pointer_motion(self->pointer_state.focus, time_msec, point);
}

void
zn_seat_pointer_axis(struct zn_seat *self, uint32_t time_msec,
    enum wlr_axis_source source, enum wlr_axis_orientation orientation,
    double delta, int32_t delta_discrete)
{
  if (self->pointer_state.focus == NULL) {
    return;
  }

  zn_snode_pointer_axis(self->pointer_state.focus, time_msec, source,
      orientation, delta, delta_discrete);
}

void
zn_seat_pointer_frame(struct zn_seat *self)
{
  if (self->pointer_state.focus == NULL) {
    return;
  }

  zn_snode_pointer_frame(self->pointer_state.focus);
}

void
zn_seat_notify_pointer_motion(
    struct zn_seat *self, struct wlr_event_pointer_motion *event)
{
  wl_signal_emit(&self->events.pointer_motion, event);
}

void
zn_seat_notify_pointer_button(
    struct zn_seat *self, struct wlr_event_pointer_button *event)
{
  if (event->state == WLR_BUTTON_PRESSED) {
    self->pointer_state.button_count++;
  } else {
    self->pointer_state.button_count--;
  }

  wl_signal_emit(&self->events.pointer_button, event);
}

void
zn_seat_notify_pointer_axis(
    struct zn_seat *self, struct wlr_event_pointer_axis *event)
{
  wl_signal_emit(&self->events.pointer_axis, event);
}

void
zn_seat_notify_pointer_frame(struct zn_seat *self)
{
  wl_signal_emit(&self->events.pointer_frame, NULL);
}

void
zn_seat_notify_capabilities(struct zn_seat *self, uint32_t capabilities)
{
  wl_signal_emit(&self->events.update_capabilities, &capabilities);
}

struct zn_seat *
zn_seat_create(struct wl_display *display)
{
  struct zn_seat *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->wlr_seat = wlr_seat_create(display, DEFAULT_SEAT);
  if (self->wlr_seat == NULL) {
    zn_error("Failed to create a wlr_seat");
    goto err_free;
  }

  self->cursor = zn_cursor_create();
  if (self->cursor == NULL) {
    zn_error("Failed to create a zn_cursor");
    goto err_seat;
  }

  self->pointer_state.focus = NULL;
  self->pointer_state.button_count = 0;

  wl_signal_init(&self->events.pointer_motion);
  wl_signal_init(&self->events.pointer_button);
  wl_signal_init(&self->events.pointer_axis);
  wl_signal_init(&self->events.pointer_frame);
  wl_signal_init(&self->events.update_capabilities);

  self->cursor_focus_destroy_listener.notify =
      zn_seat_handle_cursor_focus_destroy;
  wl_list_init(&self->cursor_focus_destroy_listener.link);

  self->focus_destroy_listener.notify = zn_seat_handle_focus_destroy;
  wl_list_init(&self->focus_destroy_listener.link);

  return self;

err_seat:
  wlr_seat_destroy(self->wlr_seat);

err_free:
  free(self);

err:
  return NULL;
}

void
zn_seat_destroy(struct zn_seat *self)
{
  wl_list_remove(&self->cursor_focus_destroy_listener.link);
  wl_list_remove(&self->focus_destroy_listener.link);
  wl_list_remove(&self->events.update_capabilities.listener_list);
  wl_list_remove(&self->events.pointer_frame.listener_list);
  wl_list_remove(&self->events.pointer_axis.listener_list);
  wl_list_remove(&self->events.pointer_button.listener_list);
  wl_list_remove(&self->events.pointer_motion.listener_list);
  zn_cursor_destroy(self->cursor);
  wlr_seat_destroy(self->wlr_seat);
  free(self);
}
