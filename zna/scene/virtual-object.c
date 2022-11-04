#include "virtual-object.h"

#include <zen-common.h>

struct zna_virtual_object*
zna_virtual_object_create(
    struct zn_virtual_object* virtual_object, struct zna_system* system)
{
  struct zna_virtual_object* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    goto err;
  }

  self->virtual_object = virtual_object;
  self->system = system;

  return self;

err:
  return NULL;
}

void
zna_virtual_object_destroy(struct zna_virtual_object* self)
{
  free(self);
}
