#include "zen/scene/view-child.h"

#include <wlr/types/wlr_output_damage.h>

#include "zen-common.h"
#include "zen/output.h"
#include "zen/scene/subsurface.h"

static void
zn_view_child_add_damage_fbox(
    struct zn_view_child *self, struct wlr_fbox *effective_box)
{
  zn_output_add_damage_box(self->view->board->screen->output, effective_box);
}

static bool
zn_view_child_is_mapped(struct zn_view_child *self)
{
  struct zn_view_child *child = self;
  while (child) {
    if (!child->mapped) return false;

    child = child->parent;
  }
  return true;
}

static void
zn_view_child_get_surface_fbox(
    struct zn_view_child *self, struct wlr_fbox *fbox)
{
  struct wlr_surface *surface = self->impl->get_wlr_surface(self);
  int sx, sy;

  self->impl->get_view_coords(self, &sx, &sy);

  fbox->x = self->view->x + sx;
  fbox->y = self->view->y + sy;
  fbox->width = surface->current.width;
  fbox->height = surface->current.height;
}

void
zn_view_child_damage_whole(struct zn_view_child *self)
{
  struct wlr_fbox fbox;

  if (!zn_view_child_is_mapped(self) || self->view->board->screen == NULL)
    return;

  zn_view_child_get_surface_fbox(self, &fbox);
  zn_view_child_add_damage_fbox(self, &fbox);
}

void
zn_view_child_damage(struct zn_view_child *self)
{
  struct wlr_surface *surface = self->impl->get_wlr_surface(self);
  struct wlr_fbox damage_box;
  pixman_region32_t damage;
  pixman_box32_t *rects;
  int rect_count;
  int sx, sy;

  if (!zn_view_child_is_mapped(self) || self->view->board->screen == NULL)
    return;

  pixman_region32_init(&damage);

  wlr_surface_get_effective_damage(surface, &damage);
  rects = pixman_region32_rectangles(&damage, &rect_count);

  self->impl->get_view_coords(self, &sx, &sy);

  for (int i = 0; i < rect_count; ++i) {
    damage_box = (struct wlr_fbox){
        .x = self->view->x + sx + rects[i].x1,
        .y = self->view->y + sy + rects[i].y1,
        .width = rects[i].x2 - rects[i].x1,
        .height = rects[i].y2 - rects[i].y1,
    };
    zn_view_child_add_damage_fbox(self, &damage_box);
  };

  pixman_region32_fini(&damage);
}

void
zn_view_child_map(struct zn_view_child *self)
{
  self->mapped = true;
  zn_view_child_damage_whole(self);
}

void
zn_view_child_unmap(struct zn_view_child *self)
{
  zn_view_child_damage_whole(self);
  self->mapped = false;
}

static void
zn_view_child_init_subsurfaces(
    struct zn_view_child *self, struct wlr_surface *surface)
{
  struct wlr_subsurface *subsurface;
  wl_list_for_each (
      subsurface, &surface->current.subsurfaces_below, current.link) {
    zn_subsurface_create(self->view, self, subsurface);
  }
  wl_list_for_each (
      subsurface, &surface->current.subsurfaces_above, current.link) {
    zn_subsurface_create(self->view, self, subsurface);
  }
}

static void
zn_view_child_handle_new_subsurface(struct wl_listener *listener, void *data)
{
  struct zn_view_child *self =
      zn_container_of(listener, self, new_subsurface_listener);
  struct wlr_subsurface *subsurface = data;

  zn_subsurface_create(self->view, self, subsurface);
}

void
zn_view_child_init(struct zn_view_child *self, struct zn_view_child *parent,
    const struct zn_view_child_impl *impl, struct zn_view *view,
    struct wlr_surface *surface)
{
  self->impl = impl;
  self->view = view;

  self->parent = parent;

  self->mapped = false;

  wl_signal_add(&surface->events.new_subsurface,  // TODO: wlr_surface
      &self->new_subsurface_listener);
  self->new_subsurface_listener.notify = zn_view_child_handle_new_subsurface;

  zn_view_child_init_subsurfaces(self, surface);
}
