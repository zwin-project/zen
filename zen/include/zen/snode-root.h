#pragma once

#include <cglm/types.h>
#include <wlr/util/box.h>

#include "zen-common/util.h"

struct zn_snode_root_interface {
  void (*damage)(void *user_data, struct wlr_fbox *fbox);
  void (*layout_position)(void *user_data, vec2 position);
};

struct zn_snode_root {
  void *user_data;                             // @nullable, @outlive if exists
  const struct zn_snode_root_interface *impl;  // @nonnull, @outlive

  struct zn_snode *node;  // @nonnull, @owning
};

UNUSED static inline void
zn_snode_root_damage(struct zn_snode_root *self, struct wlr_fbox *fbox)
{
  self->impl->damage(self->user_data, fbox);
}

UNUSED static inline void
zn_snode_root_layout_position(struct zn_snode_root *self, vec2 position)
{
  self->impl->layout_position(self->user_data, position);
}

struct zn_snode_root *zn_snode_root_create(
    void *user_data, const struct zn_snode_root_interface *implementation);

void zn_snode_root_destroy(struct zn_snode_root *self);
