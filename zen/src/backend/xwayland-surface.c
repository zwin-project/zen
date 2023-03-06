#include "xwayland-surface.h"

#include "backend.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/seat.h"
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

static bool
zn_xwayland_surface_accepts_input(void *user_data, vec2 point)
{
  struct zn_xwayland_surface *self = user_data;
  return wlr_surface_point_accepts_input(
      self->wlr_xsurface->surface, point[0], point[1]);
}

static void
zn_xwayland_surface_handle_pointer_button(void *user_data UNUSED,
    uint32_t time_msec, uint32_t button, enum wlr_button_state state)
{
  struct zn_server *server = zn_server_get_singleton();
  wlr_seat_pointer_send_button(
      server->seat->wlr_seat, time_msec, button, state);
}

static void
zn_xwayland_surface_handle_pointer_enter(void *user_data, vec2 point)
{
  struct zn_xwayland_surface *self = user_data;
  struct zn_server *server = zn_server_get_singleton();
  wlr_seat_pointer_enter(
      server->seat->wlr_seat, self->wlr_xsurface->surface, point[0], point[1]);
}

static void
zn_xwayland_surface_handle_pointer_motion(
    void *user_data UNUSED, uint32_t time_msec, vec2 point)
{
  struct zn_server *server = zn_server_get_singleton();
  wlr_seat_pointer_send_motion(
      server->seat->wlr_seat, time_msec, point[0], point[1]);
}

static void
zn_xwayland_surface_handle_pointer_leave(void *user_data UNUSED)
{
  struct zn_server *server = zn_server_get_singleton();
  wlr_seat_pointer_clear_focus(server->seat->wlr_seat);
}

static void
zn_xwayland_surface_handle_pointer_axis(void *user_data UNUSED,
    uint32_t time_msec, enum wlr_axis_source source,
    enum wlr_axis_orientation orientation, double delta, int32_t delta_discrete)
{
  struct zn_server *server = zn_server_get_singleton();
  wlr_seat_pointer_send_axis(server->seat->wlr_seat, time_msec, orientation,
      delta, delta_discrete, source);
}

static void
zn_xwayland_surface_handle_pointer_frame(void *user_data UNUSED)
{
  struct zn_server *server = zn_server_get_singleton();
  wlr_seat_pointer_send_frame(server->seat->wlr_seat);
}

static const struct zn_snode_interface snode_implementation = {
    .get_texture = zn_xwayland_surface_handle_get_texture,
    .frame = zn_xwayland_surface_handle_frame,
    .accepts_input = zn_xwayland_surface_accepts_input,
    .pointer_button = zn_xwayland_surface_handle_pointer_button,
    .pointer_enter = zn_xwayland_surface_handle_pointer_enter,
    .pointer_motion = zn_xwayland_surface_handle_pointer_motion,
    .pointer_leave = zn_xwayland_surface_handle_pointer_leave,
    .pointer_axis = zn_xwayland_surface_handle_pointer_axis,
    .pointer_frame = zn_xwayland_surface_handle_pointer_frame,
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
zn_xwayland_surface_handle_snode_position_changed(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xwayland_surface *self =
      zn_container_of(listener, self, snode_position_changed_listener);

  if (self->snode->screen == NULL) {
    return;
  }

  // TODO(@Aki-7): Use layout coords instead of screen-local effective coords
  wlr_xwayland_surface_configure(self->wlr_xsurface,
      (int16_t)self->snode->absolute_position[0],
      (int16_t)self->snode->absolute_position[1], self->wlr_xsurface->width,
      self->wlr_xsurface->height);
}

static void
zn_xwayland_surface_handle_move(struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xwayland_surface *self =
      zn_container_of(listener, self, surface_move_listener);

  zn_view_notify_move(self->view);
}

static void
zn_xwayland_surface_handle_configure(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xwayland_surface *self =
      zn_container_of(listener, self, surface_configure_listener);
  struct wlr_fbox fbox;
  struct wlr_xwayland_surface_configure_event *ev = data;

  if (!self->wlr_xsurface->mapped) {
    wlr_xwayland_surface_configure(
        self->wlr_xsurface, ev->x, ev->y, ev->width, ev->height);
    return;
  }

  zn_snode_get_fbox(self->view->snode, &fbox);

  wlr_xwayland_surface_configure(self->wlr_xsurface, (int16_t)fbox.x,
      (int16_t)fbox.y, ev->width, ev->height);
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

  vec2 new_size = {(float)self->wlr_xsurface->surface->current.width,
      (float)self->wlr_xsurface->surface->current.height};

  if (self->view->size[0] != new_size[0] ||
      self->view->size[1] != new_size[1]) {
    zn_view_notify_resized(self->view, new_size);
  }
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

  zn_view_notify_resized(self->view,
      (vec2){self->wlr_xsurface->width, self->wlr_xsurface->height});

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

  self->surface_move_listener.notify = zn_xwayland_surface_handle_move;
  wl_signal_add(
      &wlr_xsurface->events.request_move, &self->surface_move_listener);

  self->surface_commit_listener.notify = zn_xwayland_surface_handle_commit;
  wl_list_init(&self->surface_commit_listener.link);

  self->snode_position_changed_listener.notify =
      zn_xwayland_surface_handle_snode_position_changed;
  wl_signal_add(&self->snode->events.position_changed,
      &self->snode_position_changed_listener);

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
  wl_list_remove(&self->snode_position_changed_listener.link);
  wl_list_remove(&self->surface_commit_listener.link);
  wl_list_remove(&self->surface_move_listener.link);
  wl_list_remove(&self->surface_configure_listener.link);
  wl_list_remove(&self->surface_unmap_listener.link);
  wl_list_remove(&self->surface_map_listener.link);
  wl_list_remove(&self->surface_destroy_listener.link);
  zn_snode_destroy(self->snode);
  zn_view_destroy(self->view);
  free(self);
}
