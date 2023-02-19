#pragma once

#include <wlr/types/wlr_pointer.h>

#include "zen-common/util.h"

struct zn_cursor_grab;

struct zn_cursor_grab_interface {
  void (*motion_relative)(
      struct zn_cursor_grab *grab, double dx, double dy, uint32_t time_msec);
  void (*destroy)(struct zn_cursor_grab *grab);
};

struct zn_cursor_grab {
  const struct zn_cursor_grab_interface *impl;  // @nonnull, @outlive
};

UNUSED static void
zn_cursor_grab_pointer_motion(
    struct zn_cursor_grab *self, double dx, double dy, uint32_t time_msec)
{
  self->impl->motion_relative(self, dx, dy, time_msec);
}

UNUSED static void
zn_cursor_grab_destroy(struct zn_cursor_grab *self)
{
  self->impl->destroy(self);
}
