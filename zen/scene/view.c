#include "zen/scene/view.h"

#include <stdbool.h>

#include "zen-common.h"

bool
zn_view_is_mapped(struct zn_view *self)
{
  // some zn_screen has this view in zn_screen::views
  return !wl_list_empty(&self->link);
}

void
zn_view_map_to_screen(struct zn_view *self, struct zn_screen *screen)
{
  wl_list_insert(&screen->views, &self->link);
}

void
zn_view_unmap(struct zn_view *self)
{
  wl_list_remove(&self->link);
  wl_list_init(&self->link);
}

void
zn_view_init(struct zn_view *self, enum zn_view_type type,
    const struct zn_view_impl *impl)
{
  self->type = type;
  self->impl = impl;
  wl_list_init(&self->link);
}

void
zn_view_fini(struct zn_view *self)
{
  wl_list_remove(&self->link);
}
