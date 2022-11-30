#include "zen/scene/view-child.h"

#include <wlr/types/wlr_output_damage.h>

#include "zen-common.h"
#include "zen/output.h"

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

  struct wlr_fbox surface_fbox;
  zn_view_get_surface_fbox(self->view, &surface_fbox);

  double sx, sy;
  self->impl->get_toplevel_coords(self, 0, 0, &sx, &sy);

  fbox->x = surface_fbox.x + sx;
  fbox->y = surface_fbox.y + sy;
  fbox->width = surface->current.width;
  fbox->height = surface->current.height;
}

static void
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

  if (!zn_view_child_is_mapped(self) || self->view->board->screen == NULL)
    return;

  pixman_region32_init(&damage);

  wlr_surface_get_effective_damage(surface, &damage);
  rects = pixman_region32_rectangles(&damage, &rect_count);

  double sx, sy;
  self->impl->get_toplevel_coords(self, 0, 0, &sx, &sy);

  struct wlr_fbox fbox;
  zn_view_get_surface_fbox(self->view, &fbox);

  for (int i = 0; i < rect_count; ++i) {
    damage_box = (struct wlr_fbox){
        .x = fbox.x + sx + rects[i].x1,
        .y = fbox.y + sy + rects[i].y1,
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

void
zn_view_child_init(struct zn_view_child *self,
    const struct zn_view_child_impl *impl, struct zn_view *view)
{
  self->impl = impl;
  self->view = view;

  self->parent = NULL;  // TODO: handle subsurface

  self->mapped = false;
}
