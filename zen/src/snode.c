#include "zen/snode.h"

#include <cglm/vec2.h>

#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"
#include "zen/screen.h"
#include "zen/snode-root.h"

bool
zn_snode_is_focusable(struct zn_snode *self)
{
  return (self->flags & ZN_SNODE_FLAG_FOCUSABLE) == ZN_SNODE_FLAG_FOCUSABLE;
}

struct zn_snode *
zn_snode_get_focusable_parent(struct zn_snode *self)
{
  for (struct zn_snode *node = self; node; node = node->parent) {
    if (zn_snode_is_focusable(node)) {
      return node;
    }
  }

  return NULL;
}

void
zn_snode_focus(struct zn_snode *self)
{
  struct zn_snode *node = zn_snode_get_focusable_parent(self);
  if (node == NULL) {
    return;
  }

  node->impl->on_focus(node->user_data, true);
}

void
zn_snode_unfocus(struct zn_snode *self)
{
  struct zn_snode *node = zn_snode_get_focusable_parent(self);
  if (node == NULL) {
    return;
  }

  node->impl->on_focus(node->user_data, false);
}

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

void
zn_snode_noop_on_focus(void *user_data UNUSED, bool focused UNUSED)
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
    .on_focus = zn_snode_noop_on_focus,
};

void
zn_snode_damage_whole(struct zn_snode *self)
{
  struct wlr_fbox box;
  if (!self->root) {
    return;
  }

  zn_snode_get_fbox(self, &box);
  zn_snode_root_damage(self->root, &box);
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
  if (!self->root) {
    return;
  }

  struct wlr_fbox fbox = {
      .x = damage->x + self->absolute_position[0],
      .y = damage->y + self->absolute_position[1],
      .width = damage->width,
      .height = damage->height,
  };

  zn_snode_root_damage(self->root, &fbox);
}

void
zn_snode_move_front(struct zn_snode *self)
{
  if (self->parent == NULL) {
    return;
  }

  struct zn_snode *front =
      zn_container_of(self->parent->child_node_list.prev, front, link);

  zn_snode_place_above(self, front);
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
  if (self->parent == parent && glm_vec2_eqv(self->position, position)) {
    return;
  }

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
  self->root = parent ? parent->root : NULL;
  glm_vec2_copy(position, self->position);

  zn_snode_update_absolute_position(self);

  zn_snode_damage_whole(self);

  wl_signal_emit(&self->events.position_changed, NULL);
}

void
zn_snode_change_position(struct zn_snode *self, vec2 position)
{
  if (self->parent == NULL) {
    return;
  }

  if (glm_vec2_eqv(self->position, position)) {
    return;
  }

  zn_snode_damage_whole(self);

  glm_vec2_copy(position, self->position);

  zn_snode_update_absolute_position(self);

  zn_snode_damage_whole(self);

  wl_signal_emit(&self->events.position_changed, NULL);
}

static void
zn_snode_place_next_to(
    struct zn_snode *self, struct zn_snode *sibling, bool above)
{
  if (!zn_assert(self->parent, "snode should have a parent")) {
    return;
  }

  if (!zn_assert(self->parent == sibling->parent,
          "self and sibling should have the same parent")) {
    return;
  }

  if (self == sibling) {
    return;
  }

  struct wl_list *link = above ? &sibling->link : sibling->link.prev;

  if (&self->link == link->next) {
    return;
  }

  wl_list_remove(&self->link);
  wl_list_insert(link, &self->link);

  zn_snode_damage_whole(self);

  wl_signal_emit(&self->events.position_changed, NULL);
}

void
zn_snode_place_above(struct zn_snode *self, struct zn_snode *sibling)
{
  zn_snode_place_next_to(self, sibling, true);
}

void
zn_snode_place_below(struct zn_snode *self, struct zn_snode *sibling)
{
  zn_snode_place_next_to(self, sibling, false);
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

  if (self->root) {
    vec2 root_layout_position;
    zn_snode_root_layout_position(self->root, root_layout_position);

    fbox->x += root_layout_position[0];
    fbox->y += root_layout_position[1];
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
  self->root = self->parent->root;

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
  self->flags = 0;

  self->parent = NULL;
  glm_vec2_zero(self->position);
  glm_vec2_zero(self->absolute_position);

  self->root = NULL;

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
zn_snode_create_focusable(
    void *user_data, const struct zn_snode_interface *implementation)
{
  struct zn_snode *self = zn_snode_create(user_data, implementation);

  if (self) {
    self->flags |= ZN_SNODE_FLAG_FOCUSABLE;
  }

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
