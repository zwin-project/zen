#include "zen/scene/view-child.h"

void
zn_view_child_init(struct zn_view_child *self, struct zn_view_child_impl *impl,
    struct zn_view *view, struct wlr_surface *surface)
{
  self->surface = surface;
  self->impl = impl;
  self->view = view;
  wl_list_init(&self->child_list);

  self->wlr_surface_commit_listener.notify = ;
  wl_signal_add(&surface->events.commit, &self->wlr_surface_commit_listener);

  self->wlr_surface_destroy_listener.notify = ;
  wl_signal_add(&surface->events.destroy, &self->wlr_surface_destroy_listener);
}

void
zn_view_child_fini(struct zn_view_child *self)
{
}
