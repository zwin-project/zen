#include "zen-desktop/cursor-grab/default.h"

#include "zen-common/log.h"
#include "zen-common/util.h"

static void zn_cursor_default_grab_destroy(struct zn_cursor_default_grab *self);

static struct zn_cursor_default_grab *
zn_cursor_default_grab_get(struct zn_cursor_grab *grab)
{
  struct zn_cursor_default_grab *self = zn_container_of(grab, self, base);
  return self;
}

static void
zn_cursor_default_grab_handle_motion_relative(
    struct zn_cursor_grab *grab UNUSED, double dx UNUSED, double dy UNUSED,
    uint32_t time_msec UNUSED)
{}

static void
zn_cursor_default_grab_handle_destroy(struct zn_cursor_grab *grab)
{
  struct zn_cursor_default_grab *self = zn_cursor_default_grab_get(grab);
  zn_cursor_default_grab_destroy(self);
}

static const struct zn_cursor_grab_interface implementation = {
    .motion_relative = zn_cursor_default_grab_handle_motion_relative,
    .destroy = zn_cursor_default_grab_handle_destroy,
};

struct zn_cursor_default_grab *
zn_cursor_default_grab_create(void)
{
  struct zn_cursor_default_grab *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->base.impl = &implementation;

  return self;

err:
  return NULL;
}

static void
zn_cursor_default_grab_destroy(struct zn_cursor_default_grab *self)
{
  free(self);
}
