#include "zen/xwayland-view.h"

#include "zen-common.h"
#include "zen/input/cursor-grab-move.h"
#include "zen/input/cursor-grab-resize.h"
#include "zen/scene/scene.h"
#include "zen/scene/view.h"

static void zn_xwayland_view_destroy(struct zn_xwayland_view* self);

static void
zn_xwayland_view_handle_wlr_surface_commit(
    struct wl_listener* listener, void* data)
{
  struct zn_xwayland_view* self =
      zn_container_of(listener, self, wlr_surface_commit_listener);
  UNUSED(data);

  zn_view_damage(&self->base);

  // FIXME: when resizing, adjust the position of the view
}

static void
zn_xwayland_view_handle_map(struct wl_listener* listener, void* data)
{
  struct zn_xwayland_view* self = zn_container_of(listener, self, map_listener);
  struct zn_scene* scene = self->server->scene;
  UNUSED(data);

  if (!zn_assert(!zn_view_is_mapped(&self->base), "Tried to map a mapped view"))
    return;

  zn_view_map_to_scene(&self->base, scene);

  wl_list_remove(&self->move_listener.link);
  wl_list_init(&self->move_listener.link);

  wl_signal_add(
      &self->wlr_xwayland_surface->events.request_move, &self->move_listener);
  wl_signal_add(&self->wlr_xwayland_surface->events.request_resize,
      &self->resize_listener);
  wl_signal_add(&self->wlr_xwayland_surface->surface->events.commit,
      &self->wlr_surface_commit_listener);
}

static void
zn_xwayland_view_handle_unmap(struct wl_listener* listener, void* data)
{
  struct zn_xwayland_view* self =
      zn_container_of(listener, self, unmap_listener);
  UNUSED(data);

  if (!zn_assert(
          zn_view_is_mapped(&self->base), "Tried to unmap an unmapped view"))
    return;

  zn_view_unmap(&self->base);

  wl_list_remove(&self->move_listener.link);
  wl_list_init(&self->move_listener.link);
  wl_list_remove(&self->resize_listener.link);
  wl_list_init(&self->resize_listener.link);
  wl_list_remove(&self->wlr_surface_commit_listener.link);
  wl_list_init(&self->wlr_surface_commit_listener.link);
}

static void
zn_xwayland_view_handle_move(struct wl_listener* listener, void* data)
{
  // FIXME: pointer/button/serial validation
  UNUSED(data);
  struct zn_xwayland_view* self =
      zn_container_of(listener, self, move_listener);
  struct zn_server* server = zn_server_get_singleton();
  struct zn_cursor* cursor = server->input_manager->seat->cursor;

  zn_cursor_grab_move_start(cursor, &self->base);
}

static void
zn_xwayland_view_handle_resize(struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zn_xwayland_view* self =
      zn_container_of(listener, self, resize_listener);
  struct wlr_xwayland_resize_event* event = data;
  struct zn_server* server = zn_server_get_singleton();
  struct zn_cursor* cursor = server->input_manager->seat->cursor;

  zn_cursor_grab_resize_start(cursor, &self->base, event->edges);
}

static void
zn_xwayland_view_handle_wlr_xwayland_surface_destroy(
    struct wl_listener* listener, void* data)
{
  struct zn_xwayland_view* self =
      zn_container_of(listener, self, wlr_xwayland_surface_destroy_listener);
  UNUSED(data);

  zn_xwayland_view_destroy(self);
}

static void
zn_xwayland_view_handle_wlr_xwayland_surface_set_decorations(
    struct wl_listener* listener, void* data)
{
  struct zn_xwayland_view* self = zn_container_of(
      listener, self, wlr_xwayland_surface_set_decorations_listener);
  UNUSED(data);
}

static void
zn_xwayland_view_impl_set_activated(struct zn_view* view, bool active)
{
  struct zn_xwayland_view* self = zn_container_of(view, self, base);
  wlr_xwayland_surface_activate(self->wlr_xwayland_surface, active);
}

static struct wlr_surface*
zn_xwayland_view_impl_get_wlr_surface(struct zn_view* view)
{
  struct zn_xwayland_view* self = zn_container_of(view, self, base);
  return self->wlr_xwayland_surface->surface;
}

static struct wlr_surface*
zn_xwayland_view_impl_get_wlr_surface_at(struct zn_view* view, double view_sx,
    double view_sy, double* surface_x, double* surface_y)
{
  struct zn_xwayland_view* self = zn_container_of(view, self, base);

  return wlr_surface_surface_at(self->wlr_xwayland_surface->surface, view_sx,
      view_sy, surface_x, surface_y);
}

static void
zn_xwayland_view_impl_get_geometry(struct zn_view* view, struct wlr_box* box)
{
  struct zn_xwayland_view* self = zn_container_of(view, self, base);
  struct wlr_surface* surface = zn_xwayland_view_impl_get_wlr_surface(view);

  box->x = 0;
  box->y = 0;
  box->width = surface->current.width;
  box->height = surface->current.height;
}

static uint32_t
zn_xwayland_view_impl_set_size(
    struct zn_view* view, double width, double height)
{
  struct zn_xwayland_view* self = zn_container_of(view, self, base);

  wlr_xwayland_surface_configure(self->wlr_xwayland_surface,
      self->wlr_xwayland_surface->x, self->wlr_xwayland_surface->y, width,
      height);

  return 0;
}

static void
zn_xwayland_view_impl_set_position(struct zn_view* view, double x, double y)
{
  struct zn_xwayland_view* self = zn_container_of(view, self, base);

  wlr_xwayland_surface_configure(self->wlr_xwayland_surface, x, y,
      self->wlr_xwayland_surface->width, self->wlr_xwayland_surface->height);
}

static void
zn_xwayland_view_impl_restack(struct zn_view* view, enum xcb_stack_mode_t mode)
{
  struct zn_xwayland_view* self = zn_container_of(view, self, base);
  wlr_xwayland_surface_restack(self->wlr_xwayland_surface, NULL, mode);
}

static const struct zn_view_impl zn_xwayland_view_impl = {
    .get_wlr_surface = zn_xwayland_view_impl_get_wlr_surface,
    .get_wlr_surface_at = zn_xwayland_view_impl_get_wlr_surface_at,
    .get_geometry = zn_xwayland_view_impl_get_geometry,
    .set_activated = zn_xwayland_view_impl_set_activated,
    .set_size = zn_xwayland_view_impl_set_size,
    .set_position = zn_xwayland_view_impl_set_position,
    .restack = zn_xwayland_view_impl_restack,
};

struct zn_xwayland_view*
zn_xwayland_view_create(
    struct wlr_xwayland_surface* xwayland_surface, struct zn_server* server)
{
  struct zn_xwayland_view* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory.");
    wl_resource_post_no_memory(xwayland_surface->surface->resource);
    goto err;
  }

  self->server = server;

  zn_view_init(&self->base, ZN_VIEW_XWAYLAND, &zn_xwayland_view_impl);

  self->wlr_xwayland_surface = xwayland_surface;

  self->map_listener.notify = zn_xwayland_view_handle_map;
  wl_signal_add(&self->wlr_xwayland_surface->events.map, &self->map_listener);

  self->unmap_listener.notify = zn_xwayland_view_handle_unmap;
  wl_signal_add(
      &self->wlr_xwayland_surface->events.unmap, &self->unmap_listener);

  self->move_listener.notify = zn_xwayland_view_handle_move;
  wl_list_init(&self->move_listener.link);

  self->resize_listener.notify = zn_xwayland_view_handle_resize;
  wl_list_init(&self->resize_listener.link);

  self->wlr_xwayland_surface_destroy_listener.notify =
      zn_xwayland_view_handle_wlr_xwayland_surface_destroy;
  wl_signal_add(&self->wlr_xwayland_surface->events.destroy,
      &self->wlr_xwayland_surface_destroy_listener);

  self->wlr_xwayland_surface_set_decorations_listener.notify =
      zn_xwayland_view_handle_wlr_xwayland_surface_set_decorations;
  wl_signal_add(&self->wlr_xwayland_surface->events.set_decorations,
      &self->wlr_xwayland_surface_set_decorations_listener);

  self->wlr_surface_commit_listener.notify =
      zn_xwayland_view_handle_wlr_surface_commit;
  wl_list_init(&self->wlr_surface_commit_listener.link);

  return self;

err:
  return NULL;
}

static void
zn_xwayland_view_destroy(struct zn_xwayland_view* self)
{
  wl_list_remove(&self->wlr_xwayland_surface_destroy_listener.link);
  wl_list_remove(&self->wlr_xwayland_surface_set_decorations_listener.link);
  wl_list_remove(&self->wlr_surface_commit_listener.link);
  wl_list_remove(&self->move_listener.link);
  wl_list_remove(&self->resize_listener.link);
  wl_list_remove(&self->map_listener.link);
  wl_list_remove(&self->unmap_listener.link);
  zn_view_fini(&self->base);
  free(self);
}
