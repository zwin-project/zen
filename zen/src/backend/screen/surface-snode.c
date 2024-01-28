#include "surface-snode.h"

#include <cglm/vec2.h>
#include <wlr/types/wlr_compositor.h>

#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"
#include "zen/seat.h"
#include "zen/server.h"
#include "zen/snode.h"

static void zn_surface_snode_destroy(struct zn_surface_snode *self);

struct wlr_texture *
zn_surface_snode_get_texture(void *user_data)
{
  struct zn_surface_snode *self = user_data;
  return wlr_surface_get_texture(self->surface);
}

void
zn_surface_snode_frame(void *user_data, const struct timespec *when)
{
  struct zn_surface_snode *self = user_data;
  wlr_surface_send_frame_done(self->surface, when);
}

bool
zn_surface_snode_accepts_input(void *user_data UNUSED, const vec2 point UNUSED)
{
  struct zn_surface_snode *self = user_data;
  return wlr_surface_point_accepts_input(self->surface, point[0], point[1]);
}

void
zn_surface_snode_pointer_button(void *user_data UNUSED, uint32_t time_msec,
    uint32_t button, enum wlr_button_state state)
{
  struct zn_server *server = zn_server_get_singleton();
  wlr_seat_pointer_send_button(
      server->seat->wlr_seat, time_msec, button, state);
}

void
zn_surface_snode_pointer_enter(void *user_data, const vec2 point)
{
  struct zn_surface_snode *self = user_data;
  struct zn_server *server = zn_server_get_singleton();
  wlr_seat_pointer_enter(
      server->seat->wlr_seat, self->surface, point[0], point[1]);
}

void
zn_surface_snode_pointer_motion(
    void *user_data UNUSED, uint32_t time_msec, const vec2 point)
{
  struct zn_server *server = zn_server_get_singleton();
  wlr_seat_pointer_send_motion(
      server->seat->wlr_seat, time_msec, point[0], point[1]);
}

void
zn_surface_snode_pointer_leave(void *user_data UNUSED)
{
  struct zn_server *server = zn_server_get_singleton();
  wlr_seat_pointer_clear_focus(server->seat->wlr_seat);
}

void
zn_surface_snode_pointer_axis(void *user_data UNUSED, uint32_t time_msec,
    enum wlr_axis_source source, enum wlr_axis_orientation orientation,
    double delta, int32_t delta_discrete)
{
  struct zn_server *server = zn_server_get_singleton();
  wlr_seat_pointer_send_axis(server->seat->wlr_seat, time_msec, orientation,
      delta, delta_discrete, source);
}

void
zn_surface_snode_pointer_frame(void *user_data UNUSED)
{
  struct zn_server *server = zn_server_get_singleton();
  wlr_seat_pointer_send_frame(server->seat->wlr_seat);
}

static const struct zn_snode_interface implementation = {
    .get_texture = zn_surface_snode_get_texture,
    .frame = zn_surface_snode_frame,
    .accepts_input = zn_surface_snode_accepts_input,
    .pointer_button = zn_surface_snode_pointer_button,
    .pointer_enter = zn_surface_snode_pointer_enter,
    .pointer_motion = zn_surface_snode_pointer_motion,
    .pointer_leave = zn_surface_snode_pointer_leave,
    .pointer_axis = zn_surface_snode_pointer_axis,
    .pointer_frame = zn_surface_snode_pointer_frame,
    .on_focus = zn_snode_noop_on_focus,
};

typedef void (*zn_surface_snode_iter_children_func_t)(
    struct zn_surface_snode *self, struct zn_surface_snode *child, void *data);

static void
zn_surface_snode_iter_children(struct zn_surface_snode *self,
    zn_surface_snode_iter_children_func_t func, void *data)
{
  struct zn_snode *snode = NULL;

  wl_list_for_each (
      snode, &self->subsurfaces_below_node->child_node_list, link) {
    struct zn_surface_snode *surface_snode = snode->user_data;
    func(self, surface_snode, data);
  }

  wl_list_for_each (
      snode, &self->subsurfaces_above_node->child_node_list, link) {
    struct zn_surface_snode *surface_snode = snode->user_data;
    func(self, surface_snode, data);
  }
}

static void
zn_surface_snode_update_child_subsurface_position(
    struct zn_surface_snode *self UNUSED, struct zn_surface_snode *child,
    void *data UNUSED)
{
  if (!wlr_surface_is_subsurface(child->surface)) {
    return;
  }

  struct wlr_subsurface *subsurface =
      wlr_subsurface_from_wlr_surface(child->surface);

  vec2 position = {(float)subsurface->current.x, (float)subsurface->current.y};

  zn_snode_change_position(child->surface_node, position);
}

static void
zn_surface_snode_update_child_subsurfaces_position(
    struct zn_surface_snode *self)
{
  zn_surface_snode_iter_children(
      self, zn_surface_snode_update_child_subsurface_position, NULL);
}

static void
zn_surface_snode_handle_new_subsurface(
    struct wl_listener *listener UNUSED, void *user_data)
{
  struct wlr_subsurface *subsurface = user_data;

  zn_surface_snode_create(subsurface->surface);
}

static void
zn_surface_snode_handle_commit(
    struct wl_listener *listener, void *user_data UNUSED)
{
  struct zn_surface_snode *self =
      zn_container_of(listener, self, surface_commit_listener);

  zn_surface_snode_commit_damage(self);

  zn_surface_snode_update_child_subsurfaces_position(self);
}

static void
zn_surface_snode_handle_surface_destroy(
    struct wl_listener *listener, void *user_data UNUSED)
{
  struct zn_surface_snode *self =
      zn_container_of(listener, self, surface_destroy_listener);

  zn_surface_snode_destroy(self);
}

void
zn_surface_snode_commit_damage(struct zn_surface_snode *self)
{
  pixman_region32_t damage;

  pixman_region32_init(&damage);

  wlr_surface_get_effective_damage(self->surface, &damage);

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

    zn_snode_damage(self->surface_node, &damage_fbox);
  }

  pixman_region32_fini(&damage);
}

void
zn_surface_snode_damage_whole(struct zn_surface_snode *self)
{
  zn_snode_damage_whole(self->surface_node);
}

struct zn_surface_snode *
zn_surface_snode_create(struct wlr_surface *surface)
{
  struct zn_surface_snode *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->surface = surface;
  wl_signal_init(&self->events.destroy);
  surface->data = self;

  self->snode = zn_snode_create(self, &zn_snode_noop_implementation);
  if (self->snode == NULL) {
    zn_error("Failed to create a snode");
    goto err_free;
  }

  self->surface_node = zn_snode_create(self, &implementation);
  if (self->surface_node == NULL) {
    zn_error("Failed to create a surface snode");
    goto err_snode;
  }

  self->subsurfaces_above_node =
      zn_snode_create(self, &zn_snode_noop_implementation);
  if (self->subsurfaces_above_node == NULL) {
    zn_error("Failed to create an above subsurfaces node");
    goto err_surface_node;
  }

  self->subsurfaces_below_node =
      zn_snode_create(self, &zn_snode_noop_implementation);
  if (self->subsurfaces_below_node == NULL) {
    zn_error("Failed to create an below subsurface node");
    goto err_subsurface_above_node;
  }

  self->surface_destroy_listener.notify =
      zn_surface_snode_handle_surface_destroy;
  wl_signal_add(&surface->events.destroy, &self->surface_destroy_listener);

  self->new_subsurface_listener.notify = zn_surface_snode_handle_new_subsurface;
  wl_signal_add(
      &surface->events.new_subsurface, &self->new_subsurface_listener);

  self->surface_commit_listener.notify = zn_surface_snode_handle_commit;
  wl_signal_add(&self->surface->events.commit, &self->surface_commit_listener);

  zn_snode_set_position(
      self->subsurfaces_below_node, self->snode, GLM_VEC2_ZERO);
  zn_snode_set_position(self->surface_node, self->snode, GLM_VEC2_ZERO);
  zn_snode_set_position(
      self->subsurfaces_above_node, self->snode, GLM_VEC2_ZERO);

  struct wlr_subsurface *subsurface = NULL;
  wl_list_for_each (
      subsurface, &self->surface->current.subsurfaces_below, current.link) {
    struct zn_surface_snode *surface_snode =
        zn_surface_snode_create(subsurface->surface);
    zn_snode_set_position(
        surface_snode->snode, self->subsurfaces_below_node, GLM_VEC2_ZERO);
  }

  wl_list_for_each (
      subsurface, &self->surface->current.subsurfaces_above, current.link) {
    struct zn_surface_snode *surface_snode =
        zn_surface_snode_create(subsurface->surface);
    zn_snode_set_position(
        surface_snode->snode, self->subsurfaces_above_node, GLM_VEC2_ZERO);
  }

  zn_surface_snode_update_child_subsurfaces_position(self);

  return self;

err_subsurface_above_node:
  zn_snode_destroy(self->subsurfaces_above_node);

err_surface_node:
  zn_snode_destroy(self->surface_node);

err_snode:
  zn_snode_destroy(self->snode);

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_surface_snode_destroy(struct zn_surface_snode *self)
{
  zn_signal_emit_mutable(&self->events.destroy, NULL);

  zn_snode_destroy(self->subsurfaces_below_node);
  zn_snode_destroy(self->subsurfaces_above_node);
  zn_snode_destroy(self->surface_node);
  zn_snode_destroy(self->snode);
  wl_list_remove(&self->new_subsurface_listener.link);
  wl_list_remove(&self->surface_commit_listener.link);
  wl_list_remove(&self->surface_destroy_listener.link);
  wl_list_remove(&self->events.destroy.listener_list);

  free(self);
}
