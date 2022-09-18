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
zn_view_child_view_unmap_handler(struct wl_listener *listener, void *data)
{
  struct zn_view_child *self =
      zn_container_of(listener, self, view_unmap_listener);
  UNUSED(data);
  zn_view_child_unmap(self);
}

static void
zn_view_child_view_destroy_handler(struct wl_listener *listener, void *data)
{
  struct zn_view_child *self =
      zn_container_of(listener, self, view_destroy_listener);
  UNUSED(data);

  zn_view_child_unmap(self);
  self->view = NULL;
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

  if (!zn_view_child_is_mapped(self) || self->view == NULL ||
      self->view->board->screen == NULL)
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
  zn_view_child_damage(self);
}

void
zn_view_child_unmap(struct zn_view_child *self)
{
  zn_view_child_damage(self);
  self->mapped = false;
}

void
zn_view_child_init(struct zn_view_child *self,
    const struct zn_view_child_impl *impl, struct zn_view *view)
{
  self->impl = impl;
  self->view = view;
  self->parent =
      NULL;  // TODO: handle if view_child's parent(view_child) exists.

  self->mapped = false;

  self->view_unmap_listener.notify = zn_view_child_view_unmap_handler;
  wl_signal_add(&self->view->events.unmap, &self->view_unmap_listener);
  self->view_destroy_listener.notify = zn_view_child_view_destroy_handler;
  wl_signal_add(&self->view->events.destroy, &self->view_destroy_listener);
}

void
zn_view_child_fini(struct zn_view_child *self)
{
  wl_list_remove(&self->view_unmap_listener.link);
  wl_list_remove(&self->view_destroy_listener.link);
}
