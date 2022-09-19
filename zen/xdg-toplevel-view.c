#include "zen/xdg-toplevel-view.h"

#include "zen-common.h"
#include "zen/input/cursor-grab-move.h"
#include "zen/input/cursor-grab-resize.h"
#include "zen/scene/scene.h"
#include "zen/scene/screen-layout.h"
#include "zen/scene/view.h"
#include "zen/xdg-popup.h"

static void zn_xdg_toplevel_view_destroy(struct zn_xdg_toplevel_view* self);

static void
zn_xdg_toplevel_view_handle_wlr_surface_commit(
    struct wl_listener* listener, void* data)
{
  struct zn_xdg_toplevel_view* self =
      zn_container_of(listener, self, wlr_surface_commit_listener);
  UNUSED(data);

  zn_view_damage(&self->base);
}

static void
zn_xdg_toplevel_view_handle_map(struct wl_listener* listener, void* data)
{
  struct zn_xdg_toplevel_view* self =
      zn_container_of(listener, self, map_listener);
  struct zn_scene* scene = self->server->scene;
  UNUSED(data);

  if (!zn_assert(!zn_view_is_mapped(&self->base), "Tried to map a mapped view"))
    return;

  zn_view_map_to_scene(&self->base, scene);

  wl_signal_add(&self->wlr_xdg_toplevel->base->events.new_popup,
      &self->new_popup_listener);
  wl_signal_add(
      &self->wlr_xdg_toplevel->events.request_move, &self->move_listener);
  wl_signal_add(&self->wlr_xdg_toplevel->base->surface->events.commit,
      &self->wlr_surface_commit_listener);
}

static void
zn_xdg_toplevel_view_handle_unmap(struct wl_listener* listener, void* data)
{
  struct zn_xdg_toplevel_view* self =
      zn_container_of(listener, self, unmap_listener);
  UNUSED(data);

  if (!zn_assert(
          zn_view_is_mapped(&self->base), "Tried to unmap an unmapped view"))
    return;

  zn_view_unmap(&self->base);

  wl_list_remove(&self->new_popup_listener.link);
  wl_list_init(&self->new_popup_listener.link);

  wl_list_remove(&self->move_listener.link);
  wl_list_init(&self->move_listener.link);

  wl_list_remove(&self->wlr_surface_commit_listener.link);
  wl_list_init(&self->wlr_surface_commit_listener.link);
}

static void
zn_xdg_toplevel_view_handle_new_popup(struct wl_listener* listener, void* data)
{
  struct zn_xdg_toplevel_view* self =
      zn_container_of(listener, self, new_popup_listener);
  struct wlr_xdg_popup* wlr_xdg_popup = data;

  (void)zn_xdg_popup_create(wlr_xdg_popup, &self->base);
}

static void
zn_xdg_toplevel_view_handle_move(struct wl_listener* listener, void* data)
{
  // FIXME: pointer/button/serial validation
  UNUSED(data);
  struct zn_xdg_toplevel_view* self =
      zn_container_of(listener, self, move_listener);
  struct zn_server* server = zn_server_get_singleton();
  struct zn_cursor* cursor = server->input_manager->seat->cursor;

  zn_cursor_grab_move_start(cursor, &self->base);
}

static void
zn_xdg_toplevel_view_resize_handler(struct wl_listener* listener, void* data)
{
  struct zn_xdg_toplevel_view* self =
      zn_container_of(listener, self, resize_listener);
  struct wlr_xdg_toplevel_resize_event* event = data;
  struct zn_server* server = zn_server_get_singleton();
  struct zn_cursor* cursor = server->input_manager->seat->cursor;

  zn_cursor_grab_resize_start(cursor, &self->base, event->edges);
}

static void
zn_xdg_toplevel_view_handle_wlr_xdg_surface_destroy(
    struct wl_listener* listener, void* data)
{
  struct zn_xdg_toplevel_view* self =
      zn_container_of(listener, self, wlr_xdg_surface_destroy_listener);
  UNUSED(data);

  zn_xdg_toplevel_view_destroy(self);
}

static void
zn_xdg_toplevel_view_impl_set_activated(struct zn_view* view, bool active)
{
  struct zn_xdg_toplevel_view* self = zn_container_of(view, self, base);
  wlr_xdg_toplevel_set_activated(self->wlr_xdg_toplevel->base, active);
}

static struct wlr_surface*
zn_xdg_toplevel_view_impl_get_wlr_surface(struct zn_view* view)
{
  struct zn_xdg_toplevel_view* self = zn_container_of(view, self, base);
  return self->wlr_xdg_toplevel->base->surface;
}

static void
zn_xdg_toplevel_view_impl_get_geometry(
    struct zn_view* view, struct wlr_box* box)
{
  struct zn_xdg_toplevel_view* self = zn_container_of(view, self, base);
  wlr_xdg_surface_get_geometry(self->wlr_xdg_toplevel->base, box);
}

static void
zn_xdg_toplevel_view_impl_for_each_popup_surface(
    struct zn_view* view, wlr_surface_iterator_func_t iterator, void* user_data)
{
  struct zn_xdg_toplevel_view* self;

  if (!zn_assert(
          view->type == ZN_VIEW_XDG_TOPLEVEL, "Expected xdg toplevel view"))
    return;

  self = zn_container_of(view, self, base);

  wlr_xdg_surface_for_each_popup_surface(
      self->wlr_xdg_toplevel->base, iterator, user_data);
}

static void
zn_xdg_toplevel_view_impl_close_popups(struct zn_view* view)
{
  struct zn_xdg_toplevel_view* self = zn_container_of(view, self, base);
  struct wlr_xdg_popup *popup, *tmp;
  wl_list_for_each_safe(popup, tmp, &self->wlr_xdg_toplevel->base->popups, link)
  {
    wlr_xdg_popup_destroy(popup->base);
  }
}

static const struct zn_view_impl zn_xdg_toplevel_view_impl = {
    .get_wlr_surface = zn_xdg_toplevel_view_impl_get_wlr_surface,
    .get_geometry = zn_xdg_toplevel_view_impl_get_geometry,
    .set_activated = zn_xdg_toplevel_view_impl_set_activated,
    .for_each_popup_surface = zn_xdg_toplevel_view_impl_for_each_popup_surface,
    .close_popups = zn_xdg_toplevel_view_impl_close_popups,
};

struct zn_xdg_toplevel_view*
zn_xdg_toplevel_view_create(
    struct wlr_xdg_toplevel* wlr_xdg_toplevel, struct zn_server* server)
{
  struct zn_xdg_toplevel_view* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    wl_resource_post_no_memory(wlr_xdg_toplevel->resource);
    goto err;
  }

  self->server = server;

  zn_view_init(&self->base, ZN_VIEW_XDG_TOPLEVEL, &zn_xdg_toplevel_view_impl);

  self->wlr_xdg_toplevel = wlr_xdg_toplevel;

  self->map_listener.notify = zn_xdg_toplevel_view_handle_map;
  wl_signal_add(&self->wlr_xdg_toplevel->base->events.map, &self->map_listener);

  self->unmap_listener.notify = zn_xdg_toplevel_view_handle_unmap;
  wl_signal_add(
      &self->wlr_xdg_toplevel->base->events.unmap, &self->unmap_listener);

  self->new_popup_listener.notify = zn_xdg_toplevel_view_handle_new_popup;
  wl_list_init(&self->new_popup_listener.link);

  self->move_listener.notify = zn_xdg_toplevel_view_handle_move;
  wl_list_init(&self->move_listener.link);

  self->resize_listener.notify = zn_xdg_toplevel_view_resize_handler;
  wl_signal_add(
      &self->wlr_xdg_toplevel->events.request_resize, &self->resize_listener);

  self->wlr_xdg_surface_destroy_listener.notify =
      zn_xdg_toplevel_view_handle_wlr_xdg_surface_destroy;
  wl_signal_add(&wlr_xdg_toplevel->base->events.destroy,
      &self->wlr_xdg_surface_destroy_listener);

  self->wlr_surface_commit_listener.notify =
      zn_xdg_toplevel_view_handle_wlr_surface_commit;
  wl_list_init(&self->wlr_surface_commit_listener.link);

  return self;

err:
  return NULL;
}

static void
zn_xdg_toplevel_view_destroy(struct zn_xdg_toplevel_view* self)
{
  wl_list_remove(&self->wlr_surface_commit_listener.link);
  wl_list_remove(&self->wlr_xdg_surface_destroy_listener.link);
  wl_list_remove(&self->move_listener.link);
  wl_list_remove(&self->new_popup_listener.link);
  wl_list_remove(&self->unmap_listener.link);
  wl_list_remove(&self->map_listener.link);
  zn_view_fini(&self->base);
  free(self);
}
