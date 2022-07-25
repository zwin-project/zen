#include "zen-common.h"
#include "zen-immersive-backend.h"

bool
zn_immersive_backend_connect(struct zn_immersive_backend* self)
{
  UNUSED(self);
  // TODO:
  return false;
}

void
zn_immersive_backend_disconnect(struct zn_immersive_backend* self)
{
  UNUSED(self);
  // TODO:
}

struct zn_immersive_backend*
zn_immersive_backend_create()
{
  struct zn_immersive_backend* self;

  self = static_cast<struct zn_immersive_backend*>(zalloc(sizeof *self));
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  return self;

err:
  return NULL;
}

void
zn_immersive_backend_destroy(struct zn_immersive_backend* self)
{
  free(self);
}
