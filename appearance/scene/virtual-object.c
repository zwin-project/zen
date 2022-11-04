#include "virtual-object.h"

#include <zen-common.h>

struct zn_virtual_object_appearance*
zn_virtual_object_appearance_create(
    struct zn_virtual_object* virtual_object, struct zn_appearance* appearance)
{
  struct zn_virtual_object_appearance* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    goto err;
  }

  self->virtual_object = virtual_object;
  self->appearance = appearance;

  return self;

err:
  return NULL;
}

void
zn_virtual_object_appearance_destroy(struct zn_virtual_object_appearance* self)
{
  free(self);
}
