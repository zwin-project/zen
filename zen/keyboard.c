#include "zen/keyboard.h"

#include <wlr/types/wlr_seat.h>

#include "zen-common.h"

struct zn_keyboard*
zn_keyboard_create()
{
  struct zn_keyboard* self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  return self;

err:
  return NULL;
}

void
zn_keyboard_destroy(struct zn_keyboard* self)
{
  free(self);
}