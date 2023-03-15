#include "zen/snode.h"

#include <cglm/vec2.h>

#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"
#include "zen/screen.h"

struct wlr_texture *
zn_snode_noop_get_texture(void *user_data UNUSED)
{
  return NULL;
}

void
zn_snode_noop_frame(void *user_data UNUSED, const struct timespec *when UNUSED)
{}

bool
zn_snode_noop_accepts_input(void *user_data UNUSED, const vec2 point UNUSED)
{
  return false;
}

void
zn_snode_noop_pointer_button(void *user_data UNUSED, uint32_t time_msec UNUSED,
    uint32_t button UNUSED, enum wlr_button_state state UNUSED)
{}

void
zn_snode_noop_pointer_enter(void *user_data UNUSED, const vec2 point UNUSED)
{}

void
zn_snode_noop_pointer_motion(
    void *user_data UNUSED, uint32_t time_msec UNUSED, const vec2 point UNUSED)
{}

void
zn_snode_noop_pointer_leave(void *user_data UNUSED)
{}

void
zn_snode_noop_pointer_axis(void *user_data UNUSED, uint32_t time_msec UNUSED,
    enum wlr_axis_source source UNUSED,
    enum wlr_axis_orientation orientation UNUSED, double delta UNUSED,
    int32_t delta_discrete UNUSED)
{}

void
zn_snode_noop_pointer_frame(void *user_data UNUSED)
{}

const struct zn_snode_interface zn_snode_noop_implementation = {
    .get_texture = zn_snode_noop_get_texture,
    .frame = zn_snode_noop_frame,
    .accepts_input = zn_snode_noop_accepts_input,
    .pointer_button = zn_snode_noop_pointer_button,
    .pointer_enter = zn_snode_noop_pointer_enter,
    .pointer_motion = zn_snode_noop_pointer_motion,
    .pointer_leave = zn_snode_noop_pointer_leave,
    .pointer_axis = zn_snode_noop_pointer_axis,
    .pointer_frame = zn_snode_noop_pointer_frame,
};

void
zn_snode_damage_whole(struct zn_snode *self)
{
  struct wlr_fbox box;
  if (!self->screen) {
    return;
  }

  zn_snode_get_fbox(self, &box);
  zn_screen_damage(self->screen, &box);
}

static void
zn_snode_update_absolute_position(struct zn_snode *self)
{
  if (self->parent) {
    glm_vec2_add(self->parent->absolute_position, self->position,
        self->absolute_position);
  } else {
    glm_vec2_copy(self->position, self->absolute_position);
  }
}

struct zn_snode *
zn_snode_get_snode_at(struct zn_snode *self, vec2 point, vec2 local_point)
{
  struct zn_snode *child = NULL;
  wl_list_for_each_reverse (child, &self->child_node_list, link) {
    vec2 child_local_point;
    glm_vec2_sub(point, child->position, child_local_point);

    struct zn_snode *result =
        zn_snode_get_snode_at(child, child_local_point, local_point);
    if (result) {
      return result;
    }
  }

  if (self->impl->accepts_input(self->user_data, point)) {
    glm_vec2_copy(point, local_point);
    return self;
  }

  return NULL;
}

void
zn_snode_damage(struct zn_snode *self, struct wlr_fbox *damage)
{
  if (!self->screen) {
    return;
  }

  struct wlr_fbox fbox = {
      .x = damage->x + self->absolute_position[0],
      .y = damage->y + self->absolute_position[1],
      .width = damage->width,
      .height = damage->height,
  };

  zn_screen_damage(self->screen, &fbox);
}

void
zn_snode_notify_frame(struct zn_snode *self, const struct timespec *when)
{
  self->impl->frame(self->user_data, when);
  struct zn_snode *snode = NULL;
  wl_list_for_each (snode, &self->child_node_list, link) {
    zn_snode_notify_frame(snode, when);
  }
}

void
zn_snode_set_position(
    struct zn_snode *self, struct zn_snode *parent, vec2 position)
{
  zn_snode_damage_whole(self);

  if (self->parent) {
    wl_list_remove(&self->link);
    wl_list_init(&self->link);
    wl_list_remove(&self->parent_destroy_listener.link);
    wl_list_init(&self->parent_destroy_listener.link);
    wl_list_remove(&self->parent_position_changed_listener.link);
    wl_list_init(&self->parent_position_changed_listener.link);
  }

  if (parent) {
    wl_list_insert(parent->child_node_list.prev, &self->link);
    wl_signal_add(&parent->events.destroy, &self->parent_destroy_listener);
    wl_signal_add(&parent->events.position_changed,
        &self->parent_position_changed_listener);
  }

  self->parent = parent;
  self->screen = parent ? parent->screen : NULL;
  glm_vec2_copy(position, self->position);

  zn_snode_update_absolute_position(self);

  zn_snode_damage_whole(self);

  wl_signal_emit(&self->events.position_changed, NULL);
}

struct wlr_texture *
zn_snode_get_texture(struct zn_snode *self)
{
  return self->impl->get_texture(self->user_data);
}

void
zn_snode_get_fbox(struct zn_snode *self, struct wlr_fbox *fbox)
{
  struct wlr_texture *texture = self->impl->get_texture(self->user_data);
  fbox->x = self->absolute_position[0];
  fbox->y = self->absolute_position[1];
  if (texture) {
    fbox->width = texture->width;
    fbox->height = texture->height;
  } else {
    fbox->width = 0;
    fbox->height = 0;
  }
}

void
zn_snode_get_layout_fbox(struct zn_snode *self, struct wlr_fbox *fbox)
{
  zn_snode_get_fbox(self, fbox);

  if (self->screen) {
    fbox->x += self->screen->layout_position[0];
    fbox->y += self->screen->layout_position[1];
  }
}

static void
zn_snode_handle_parent_position_changed(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_snode *self =
      zn_container_of(listener, self, parent_position_changed_listener);

  zn_snode_damage_whole(self);

  zn_snode_update_absolute_position(self);
  self->screen = self->parent->screen;

  zn_snode_damage_whole(self);

  wl_signal_emit(&self->events.position_changed, NULL);
}

static void
zn_snode_handle_parent_destroy(struct wl_listener *listener, void *data UNUSED)
{
  struct zn_snode *self =
      zn_container_of(listener, self, parent_destroy_listener);

  zn_snode_set_position(self, NULL, GLM_VEC2_ZERO);
}

struct zn_snode *
zn_snode_create(
    void *user_data, const struct zn_snode_interface *implementation)
{
  struct zn_snode *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->user_data = user_data;
  self->impl = implementation;

  self->parent = NULL;
  glm_vec2_zero(self->position);
  glm_vec2_zero(self->absolute_position);

  self->screen = NULL;

  wl_list_init(&self->child_node_list);
  wl_list_init(&self->link);

  self->parent_destroy_listener.notify = zn_snode_handle_parent_destroy;
  wl_list_init(&self->parent_destroy_listener.link);

  self->parent_position_changed_listener.notify =
      zn_snode_handle_parent_position_changed;
  wl_list_init(&self->parent_position_changed_listener.link);

  wl_signal_init(&self->events.position_changed);
  wl_signal_init(&self->events.destroy);

  return self;

err:
  return NULL;
}

struct zn_snode *
zn_snode_create_root(struct zn_screen *screen)
{
  struct zn_snode *self = zn_snode_create(NULL, &zn_snode_noop_implementation);
  self->screen = screen;
  return self;
}

void
zn_snode_destroy(struct zn_snode *self)
{
  zn_signal_emit_mutable(&self->events.destroy, NULL);

  wl_list_remove(&self->events.destroy.listener_list);
  wl_list_remove(&self->events.position_changed.listener_list);
  wl_list_remove(&self->parent_position_changed_listener.link);
  wl_list_remove(&self->parent_destroy_listener.link);
  wl_list_remove(&self->link);
  wl_list_remove(&self->child_node_list);
  free(self);
}
