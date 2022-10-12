#include "zen/scene/subsurface.h"

#include "zen-common.h"
#include "zen/scene/view-child.h"

static void zn_subsurface_destroy(struct zn_subsurface* self);

static void
zn_subsurface_handle_wlr_subsurface_destroy(
    struct wl_listener* listener, void* data)
{
  struct zn_subsurface* self =
      zn_container_of(listener, self, wlr_subsurface_destroy_listener);
  UNUSED(data);

  zn_subsurface_destroy(self);
}

static struct wlr_surface*
zn_subsurface_view_child_impl_get_wlr_surface(struct zn_view_child* child)
{
  struct zn_subsurface* self = zn_container_of(child, self, base);
  return self->wlr_subsurface->surface;
}

static void
zn_subsurface_view_child_impl_get_view_coords(
    struct zn_view_child* child, int* sx, int* sy)
{
  struct zn_subsurface* self = zn_container_of(child, self, base);

  if (child->parent) {
    child->parent->impl->get_view_coords(child->parent, sx, sy);
  } else {
    *sx = *sy = 0;
  }

  *sx += self->wlr_subsurface->current.x;
  *sy += self->wlr_subsurface->current.y;
}

static const struct zn_view_child_impl zn_subsurface_view_child_impl = {
    .get_wlr_surface = zn_subsurface_view_child_impl_get_wlr_surface,
    .get_view_coords = zn_subsurface_view_child_impl_get_view_coords,
};

struct zn_subsurface*
zn_subsurface_create(struct zn_view* view, struct zn_view_child* parent,
    struct wlr_subsurface* wlr_subsurface)
{
  zn_info("subsurface created!");

  struct zn_subsurface* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    wl_resource_post_no_memory(wlr_subsurface->resource);
    goto err;
  }

  zn_view_child_init(&self->base, parent, &zn_subsurface_view_child_impl, view,
      wlr_subsurface->surface);

  self->wlr_subsurface = wlr_subsurface;

  self->wlr_subsurface_destroy_listener.notify =
      zn_subsurface_handle_wlr_subsurface_destroy;
  wl_signal_add(
      &wlr_subsurface->events.destroy, &self->wlr_subsurface_destroy_listener);

  wl_list_init(&self->link);
  wl_list_insert(&view->subsurface_list, &self->link);

  self->base.mapped = true;  // TODO: ここは理由がわかったらやる

  zn_view_child_damage_whole(&self->base);

  return self;

err:
  return NULL;
}

static void
zn_subsurface_destroy(struct zn_subsurface* self)
{
  wl_list_remove(&self->wlr_subsurface_destroy_listener.link);
  wl_list_remove(&self->link);
  free(self);
}
