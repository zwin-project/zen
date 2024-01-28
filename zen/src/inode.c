#include "inode.h"

#include <cglm/quat.h>
#include <cglm/vec3.h>

#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"

void
zn_inode_noop_mapped(void *impl_data UNUSED)
{}

void
zn_inode_noop_unmapped(void *impl_data UNUSED)
{}

void
zn_inode_noop_activated(void *impl_data UNUSED)
{}

void
zn_inode_noop_deactivated(void *impl_data UNUSED)
{}

void
zn_inode_noop_moved(void *impl_data UNUSED)
{}

const struct zn_inode_interface zn_inode_noop_implementation = {
    .mapped = zn_inode_noop_mapped,
    .unmapped = zn_inode_noop_unmapped,
    .activated = zn_inode_noop_activated,
    .deactivated = zn_inode_noop_deactivated,
    .moved = zn_inode_noop_moved,
};

static void
zn_inode_map_internal(struct zn_inode *self, bool mapped)
{
  if (self->mapped == mapped) {
    return;
  }

  struct zn_inode *child = NULL;

  self->mapped = mapped;

  wl_list_for_each (child, &self->child_list, link) {
    zn_inode_map_internal(child, mapped);
  }

  if (self->mapped) {
    self->impl->mapped(self->impl_data);
  } else {
    self->impl->unmapped(self->impl_data);
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
zn_inode_set_xr_system_internal(
    struct zn_inode *self, struct zn_xr_system *xr_system)
{
  if (self->xr_system == xr_system) {
    return;
  }

  bool was_active = zn_inode_is_active(self);

  self->xr_system = xr_system;

  struct zn_inode *child = NULL;

  wl_list_for_each (child, &self->child_list, link) {
    zn_inode_set_xr_system_internal(child, xr_system);
  }

  bool is_active = zn_inode_is_active(self);

  if (was_active != is_active) {
    if (is_active) {
      self->impl->activated(self->impl_data);
    } else {
      self->impl->deactivated(self->impl_data);
    }
  }
}

void
zn_inode_set_xr_system(struct zn_inode *self, struct zn_xr_system *xr_system)
{
  zn_assert(self->parent == NULL, "self must be root");

  zn_inode_set_xr_system_internal(self, xr_system);
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

  self->impl->moved(self->impl_data);
}

void
zn_inode_move(struct zn_inode *self, struct zn_inode *parent, vec3 position,
    versor quaternion)
{
  glm_vec3_copy(position, self->position);
  glm_quat_copy(quaternion, self->quaternion);

  if (self->parent != parent) {
    if (self->parent) {
      wl_list_remove(&self->link);
      wl_list_init(&self->link);
    }

    self->parent = parent;

    if (self->parent) {
      wl_list_insert(&self->parent->child_list, &self->link);
    }

    zn_inode_set_xr_system_internal(self, parent ? parent->xr_system : NULL);

    zn_inode_map_internal(self, parent ? parent->mapped : false);
  }

  zn_inode_update_position_recursive(
      self, self->parent ? self->parent->transform_abs : GLM_MAT4_IDENTITY);
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
  self->xr_system = NULL;
  self->mapped = false;
  glm_vec3_zero(self->position);
  glm_quat_identity(self->quaternion);
  glm_mat4_identity(self->transform_abs);
  self->parent = NULL;
  wl_list_init(&self->link);
  wl_list_init(&self->child_list);

  return self;

err:
  return NULL;
}

void
zn_inode_destroy(struct zn_inode *self)
{
  zn_assert(wl_list_empty(&self->child_list),
      "Destroy child inodes before the parent inode");

  wl_list_remove(&self->child_list);
  wl_list_remove(&self->link);
  free(self);
}
