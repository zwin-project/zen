#include "zen/scene/view.h"

#include <stdbool.h>

#include "zen-common.h"
#include "zen/seat.h"
#include "zen/xdg-toplevel-view.h"
#include "zen/xwayland-view.h"

void
zn_view_get_box(struct zn_view *self, struct wlr_box *box)
{
  struct wlr_surface *surface = self->impl->get_wlr_surface(self);

  box->x = self->x;
  box->y = self->y;
  box->width = surface->current.width;
  box->height = surface->current.height;
}

bool
zn_view_is_mapped(struct zn_view *self)
{
  // some zn_screen has this view in zn_screen::views
  return !wl_list_empty(&self->link);
}

void
zn_view_focus(struct zn_view *self)
{
  struct zn_server *server = zn_server_get_singleton();
  struct wlr_surface *current_surface =
      server->input_manager->seat->wlr_seat->pointer_state.focused_surface;
  struct zn_xdg_toplevel_view *xdg_toplevel_view;
  struct zn_xwayland_view *xwayland_view;

  switch (self->type) {
    case ZN_VIEW_XDG_TOPLEVEL:
      xdg_toplevel_view = zn_container_of(self, xdg_toplevel_view, base);
      if (xdg_toplevel_view->wlr_xdg_toplevel->base->surface !=
          current_surface) {
        zn_xdg_toplevel_view_focus(xdg_toplevel_view);
      }
      break;
    case ZN_VIEW_XWAYLAND:
      xwayland_view = zn_container_of(self, xwayland_view, base);
      if (xwayland_view->wlr_xwayland_surface->surface != current_surface) {
        zn_xwayland_view_focus(xwayland_view);
      }
      break;
  }
}

void
zn_view_map_to_screen(struct zn_view *self, struct zn_screen *screen)
{
  struct wlr_box screen_box, view_box;
  zn_screen_get_box(screen, &screen_box);
  zn_view_get_box(self, &view_box);

  self->x = (screen_box.width / 2) - (view_box.width / 2);
  self->y = (screen_box.height / 2) - (view_box.height / 2);

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
