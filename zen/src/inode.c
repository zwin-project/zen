#include "zen/inode.h"

#include <cglm/quat.h>
#include <cglm/vec3.h>

#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"

static void
zn_inode_noop_mapped(void *impl_data UNUSED)
{}

static void
zn_inode_noop_unmapped(void *impl_data UNUSED)
{}

static void
zn_inode_noop_moved(void *impl_data UNUSED)
{}

const struct zn_inode_interface zn_inode_noop_implementation = {
    .mapped = zn_inode_noop_mapped,
    .unmapped = zn_inode_noop_unmapped,
    .moved = zn_inode_noop_moved,
};

static void
zn_inode_map_internal(struct zn_inode *self, bool mapped)
{
  if (self->mapped == mapped) {
    return;
  }

  struct zn_inode *child = NULL;

  wl_list_for_each (child, &self->child_list, link) {
    zn_inode_map_internal(child, mapped);
  }

  self->mapped = mapped;

  if (self->mapped) {
    self->impl->mapped(self);
  } else {
    self->impl->unmapped(self);
  }
}

void
zn_inode_map(struct zn_inode *self)
{
  zn_assert(self->parent == NULL, "self must be root");

  zn_inode_map_internal(self, true);
}

void
zn_inode_unmap(struct zn_inode *self)
{
  zn_assert(self->parent == NULL, "self must be root");

  zn_inode_map_internal(self, false);
}

static void
zn_inode_update_position_recursive(struct zn_inode *self, mat4 parent_transform)
{
  mat4 transform;
  glm_quat_mat4(self->quaternion, transform);
  transform[3][0] = self->position[0];
  transform[3][1] = self->position[1];
  transform[3][2] = self->position[2];

  glm_mul(parent_transform, transform, self->transform_abs);

  struct zn_inode *child = NULL;

  wl_list_for_each (child, &self->child_list, link) {
    zn_inode_update_position_recursive(child, self->transform_abs);
  }

  self->impl->moved(self);
}

void
zn_inode_move(struct zn_inode *self, struct zn_inode *parent, vec3 position,
    versor quaternion)
{
  glm_vec3_copy(position, self->position);
  glm_quat_copy(quaternion, self->quaternion);

  if (self->parent != parent) {
    if (self->parent) {
      wl_list_remove(&self->parent_destroy_listener.link);
      wl_list_init(&self->parent_destroy_listener.link);
      wl_list_remove(&self->link);
      wl_list_init(&self->link);
    }

    self->parent = parent;

    if (self->parent) {
      wl_signal_add(
          &self->parent->events.destroy, &self->parent_destroy_listener);
      wl_list_insert(&self->parent->child_list, &self->link);
    }

    zn_inode_map_internal(self, parent && parent->mapped);
  }

  zn_inode_update_position_recursive(
      self, self->parent ? self->parent->transform_abs : GLM_MAT4_IDENTITY);
}

static void
zn_inode_handle_parent_destroy(struct wl_listener *listener, void *data UNUSED)
{
  struct zn_inode *self =
      zn_container_of(listener, self, parent_destroy_listener);

  zn_inode_move(self, NULL, self->position, self->quaternion);
}

struct zn_inode *
zn_inode_create(
    void *impl_data, const struct zn_inode_interface *implementation)
{
  struct zn_inode *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->impl_data = impl_data;
  self->impl = implementation;
  glm_vec3_zero(self->position);
  glm_quat_identity(self->quaternion);
  glm_mat4_identity(self->transform_abs);
  self->parent = NULL;
  wl_list_init(&self->link);
  wl_list_init(&self->child_list);
  self->mapped = false;
  wl_signal_init(&self->events.destroy);

  self->parent_destroy_listener.notify = zn_inode_handle_parent_destroy;
  wl_list_init(&self->parent_destroy_listener.link);

  return self;

err:
  return NULL;
}

void
zn_inode_destroy(struct zn_inode *self)
{
  zn_signal_emit_mutable(&self->events.destroy, NULL);

  wl_list_remove(&self->events.destroy.listener_list);
  wl_list_remove(&self->parent_destroy_listener.link);
  wl_list_remove(&self->child_list);
  wl_list_remove(&self->link);
  free(self);
}
