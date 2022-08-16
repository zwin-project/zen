#include "zen/scene/view.h"

#include <stdbool.h>

#include "zen-common.h"
#include "zen/seat.h"
#include "zen/xdg-toplevel-view.h"
#include "zen/xwayland-view.h"

void
zn_view_get_fbox(struct zn_view *self, struct wlr_fbox *fbox)
{
  struct wlr_surface *surface = self->impl->get_wlr_surface(self);

  fbox->x = self->x;
  fbox->y = self->y;
  fbox->width = surface->current.width;
  fbox->height = surface->current.height;
}

bool
zn_view_is_mapped(struct zn_view *self)
{
  // some zn_screen has this view in zn_screen::views
  return !wl_list_empty(&self->link);
}

void
zn_view_map_to_screen(struct zn_view *self, struct zn_screen *screen)
{
  struct wlr_box screen_box;
  struct wlr_fbox view_fbox;
  zn_screen_get_box(screen, &screen_box);
  zn_view_get_fbox(self, &view_fbox);

  self->x = (screen_box.width / 2) - (view_fbox.width / 2);
  self->y = (screen_box.height / 2) - (view_fbox.height / 2);

  // List of mapped zn_view in z-order, from bottom to top
  wl_list_insert(screen->views.prev, &self->link);
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
