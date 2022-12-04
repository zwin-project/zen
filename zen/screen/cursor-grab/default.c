#include "zen/screen/cursor-grab/default.h"

#include <zen-common.h>

void
zn_default_cursor_grab_motion_relative(
    struct zn_cursor_grab *grab, double dx, double dy, uint32_t time_msec)
{
  struct zn_cursor *cursor = grab->cursor;

  zn_cursor_move(cursor, cursor->board, cursor->x + dx, cursor->y + dy);

  UNUSED(time_msec);
}

void
zn_default_cursor_grab_motion_absolute(struct zn_cursor_grab *grab,
    struct zn_board *board, double x, double y, uint32_t time_msec)
{
  zn_cursor_move(grab->cursor, board, x, y);

  UNUSED(time_msec);
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
