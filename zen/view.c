#include "zen/view.h"

#include "zen-common.h"

void
zn_view_init(struct zn_view *self, enum zn_view_type type)
{
  self->type = type;
}

void
zn_view_fini(struct zn_view *self)
{
  UNUSED(self);
  // nothing to do for now
}
