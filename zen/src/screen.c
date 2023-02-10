#include "zen/screen.h"

#include "zen-common/log.h"
#include "zen-common/util.h"

struct zn_screen *
zn_screen_create(void *impl)
{
  struct zn_screen *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->impl = impl;

  return self;

err:
  return NULL;
}

void
zn_screen_destroy(struct zn_screen *self)
{
  free(self);
}
