#include "xwayland-surface.h"

#include "backend.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/server.h"
#include "zen/snode.h"
#include "zen/view.h"

static void zn_xwayland_surface_destroy(struct zn_xwayland_surface *self);

static struct wlr_texture *
zn_xwayland_surface_handle_get_texture(void *user_data)
{
  struct zn_xwayland_surface *self = user_data;
  return wlr_surface_get_texture(self->wlr_xsurface->surface);
}

static void
zn_xwayland_surface_handle_frame(void *user_data, const struct timespec *when)
{
  struct zn_xwayland_surface *self = user_data;

  wlr_surface_send_frame_done(self->wlr_xsurface->surface, when);
}

static const struct zn_snode_interface snode_implementation = {
    .get_texture = zn_xwayland_surface_handle_get_texture,
    .frame = zn_xwayland_surface_handle_frame,
};

static void
zn_xwayland_surface_set_activated(void *impl_data UNUSED, bool activated UNUSED)
{
  // TODO(@Aki-7): implement or remove this
}

static const struct zn_view_interface view_implementation = {
    .set_activated = zn_xwayland_surface_set_activated,
};

static void
zn_xwayland_surface_handle_configure(struct wl_listener *listener, void *data)
{
  struct zn_xwayland_surface *self =
      zn_container_of(listener, self, surface_configure_listener);
  struct wlr_xwayland_surface_configure_event *ev = data;

  if (!self->wlr_xsurface->mapped) {
    wlr_xwayland_surface_configure(
        self->wlr_xsurface, ev->x, ev->y, ev->width, ev->height);
    return;
  }

  // TODO(@Aki-7): when mapped
}

static void
zn_xwayland_surface_handle_commit(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xwayland_surface *self =
      zn_container_of(listener, self, surface_commit_listener);
  pixman_region32_t damage;
  pixman_region32_init(&damage);
  wlr_surface_get_effective_damage(self->wlr_xsurface->surface, &damage);

  pixman_box32_t *rects = NULL;
  int rect_count = 0;

  rects = pixman_region32_rectangles(&damage, &rect_count);

  for (int i = 0; i < rect_count; i++) {
    struct wlr_fbox damage_fbox = {
        .x = rects[i].x1,
        .y = rects[i].y1,
        .width = rects[i].x2 - rects[i].x1,
        .height = rects[i].y2 - rects[i].y1,
    };

    zn_snode_damage(self->snode, &damage_fbox);
  }

  pixman_region32_fini(&damage);
}

static void
zn_xwayland_surface_handle_surface_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xwayland_surface *self =
      zn_container_of(listener, self, surface_destroy_listener);

  zn_xwayland_surface_destroy(self);
}

static void
zn_xwayland_surface_handle_surface_map(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xwayland_surface *self =
      zn_container_of(listener, self, surface_map_listener);
  struct zn_server *server = zn_server_get_singleton();
  struct zn_default_backend *backend = zn_default_backend_get(server->backend);

  wl_signal_add(&self->wlr_xsurface->surface->events.commit,
      &self->surface_commit_listener);

  zn_default_backend_notify_view_mapped(backend, self->view);
}

static void
zn_xwayland_surface_handle_surface_unmap(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xwayland_surface *self =
      zn_container_of(listener, self, surface_unmap_listener);

  wl_list_remove(&self->surface_commit_listener.link);
  wl_list_init(&self->surface_commit_listener.link);

  zn_view_notify_unmap(self->view);
}

struct zn_xwayland_surface *
zn_xwayland_surface_create(struct wlr_xwayland_surface *wlr_xsurface)
{
  struct zn_xwayland_surface *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->wlr_xsurface = wlr_xsurface;

  self->view = zn_view_create(self, &view_implementation);
  if (self->view == NULL) {
    zn_error("Failed to create a zn_view");
    goto err_free;
  }

  self->snode = zn_snode_create(self, &snode_implementation);
  if (self->snode == NULL) {
    zn_error("Failed to create a zn_snode");
    goto err_view;
  }

  zn_snode_set_position(self->snode, self->view->snode, (vec2){0, 0});

  self->surface_destroy_listener.notify =
      zn_xwayland_surface_handle_surface_destroy;
  wl_signal_add(&wlr_xsurface->events.destroy, &self->surface_destroy_listener);

  self->surface_map_listener.notify = zn_xwayland_surface_handle_surface_map;
  wl_signal_add(&wlr_xsurface->events.map, &self->surface_map_listener);

  self->surface_unmap_listener.notify =
      zn_xwayland_surface_handle_surface_unmap;
  wl_signal_add(&wlr_xsurface->events.unmap, &self->surface_unmap_listener);

  self->surface_configure_listener.notify =
      zn_xwayland_surface_handle_configure;
  wl_signal_add(&wlr_xsurface->events.request_configure,
      &self->surface_configure_listener);

  self->surface_commit_listener.notify = zn_xwayland_surface_handle_commit;
  wl_list_init(&self->surface_commit_listener.link);

  return self;

err_view:
  zn_view_destroy(self->view);

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_xwayland_surface_destroy(struct zn_xwayland_surface *self)
{
  wl_list_remove(&self->surface_commit_listener.link);
  wl_list_remove(&self->surface_configure_listener.link);
  wl_list_remove(&self->surface_unmap_listener.link);
  wl_list_remove(&self->surface_map_listener.link);
  wl_list_remove(&self->surface_destroy_listener.link);
  zn_snode_destroy(self->snode);
  zn_view_destroy(self->view);
  free(self);
}
