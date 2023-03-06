#pragma once

#include <cglm/types.h>
#include <wlr/types/wlr_pointer.h>

#include "zen-common/util.h"

struct zn_cursor_grab;

struct zn_cursor_grab_interface {
  void (*motion_relative)(
      struct zn_cursor_grab *grab, vec2 delta, uint32_t time_msec);
  void (*button)(struct zn_cursor_grab *grab, uint32_t time_msec,
      uint32_t button, enum wlr_button_state state);
  void (*axis)(struct zn_cursor_grab *grab, uint32_t time_msec,
      enum wlr_axis_source source, enum wlr_axis_orientation orientation,
      double delta, int32_t delta_discrete);
  void (*frame)(struct zn_cursor_grab *grab);
  void (*destroy)(struct zn_cursor_grab *grab);
};

struct zn_cursor_grab {
  const struct zn_cursor_grab_interface *impl;  // @nonnull, @outlive
};

void zn_cursor_grab_move_relative(vec2 delta);

UNUSED static void
zn_cursor_grab_pointer_motion(
    struct zn_cursor_grab *self, vec2 delta, uint32_t time_msec)
{
  self->impl->motion_relative(self, delta, time_msec);
}

UNUSED static void
zn_cursor_grab_pointer_button(struct zn_cursor_grab *self, uint32_t time_msec,
    uint32_t button, enum wlr_button_state state)
{
  self->impl->button(self, time_msec, button, state);
}

UNUSED static void
zn_cursor_grab_pointer_axis(struct zn_cursor_grab *self, uint32_t time_msec,
    enum wlr_axis_source source, enum wlr_axis_orientation orientation,
    double delta, int32_t delta_discrete)
{
  self->impl->axis(self, time_msec, source, orientation, delta, delta_discrete);
}

UNUSED static void
zn_cursor_grab_pointer_frame(struct zn_cursor_grab *self)
{
  self->impl->frame(self);
}

UNUSED static void
zn_cursor_grab_destroy(struct zn_cursor_grab *self)
{
  self->impl->destroy(self);
}
