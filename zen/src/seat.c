#include "seat.h"

#include "cursor.h"
#include "zen-common/log.h"
#include "zen-common/util.h"

struct zn_seat *
zn_seat_create(void)
{
  struct zn_seat *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  struct zn_cursor_impl *cursor = zn_cursor_impl_create();
  if (cursor == NULL) {
    zn_error("Failed to create a zn_cursor_impl");
    goto err_free;
  }
  self->cursor = &cursor->base;

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_seat_destroy(struct zn_seat *self)
{
  zn_cursor_impl_destroy(zn_cursor_impl_get(self->cursor));
  free(self);
}
