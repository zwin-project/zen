#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>

#include "zen-common/util.h"

struct zn_xr_system;

struct zn_inode_interface {
  void (*mapped)(void *impl_data);
  void (*unmapped)(void *impl_data);
  void (*activated)(void *impl_data);
  void (*deactivated)(void *impl_data);
  void (*moved)(void *impl_data);
};

void zn_inode_noop_mapped(void *impl_data);

void zn_inode_noop_unmapped(void *impl_data);

void zn_inode_noop_activated(void *impl_data);

void zn_inode_noop_deactivated(void *impl_data);

void zn_inode_noop_moved(void *impl_data);

extern const struct zn_inode_interface zn_inode_noop_implementation;

/// inode is a root node when self->parent == NULL
struct zn_inode {
  void *impl_data;                        // @nullable, @outlive
  const struct zn_inode_interface *impl;  // @nonnull, @outlive

  struct zn_xr_system *xr_system;  // @nullable, @ref

  bool mapped;

  // local coords
  vec3 position;
  versor quaternion;

  // root node local coords, affine matrix
  mat4 transform_abs;

  struct zn_inode *parent;    // @nullable, @ref
  struct wl_list link;        // zn_inode::child_list
  struct wl_list child_list;  // zn_inode::link
};

UNUSED static inline bool
zn_inode_is_mapped(struct zn_inode *self)
{
  return self->mapped;
}

UNUSED static inline bool
zn_inode_is_active(struct zn_inode *self)
{
  return self->xr_system != NULL;
}

void zn_inode_move(struct zn_inode *self, struct zn_inode *parent,
    vec3 position, versor quaternion);

/// @param self must be root
void zn_inode_unmap(struct zn_inode *self);

/// @param self must be root
void zn_inode_map(struct zn_inode *self);

struct zn_inode *zn_inode_create(
    void *impl_data, const struct zn_inode_interface *implementation);

/// Destroy child inodes before the parent inode
void zn_inode_destroy(struct zn_inode *self);
