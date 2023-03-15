#pragma once

#include <cglm/types.h>
#include <time.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/util/box.h>

struct zn_snode;

struct zn_screen_interface {
  /// @param damage_fbox : Effective coordinate system
  void (*damage)(void *impl_data, struct wlr_fbox *damage_fbox);

  struct zn_snode *(*get_layer)(
      void *impl_data, enum zwlr_layer_shell_v1_layer layer);
};

struct zn_screen {
  void *impl_data;                         // @nullable, @outlive if exists
  const struct zn_screen_interface *impl;  // @nonnull, @outlive

  void *user_data;  // @nonnull, @outlive if exists

  struct zn_snode *snode_root;  // @nonnull, @owning

  // These layers do not have a parent at first, user must set a parent to show
  // them. User must not add children to these layers.
  // Index is corresponding to enum zwlr_layer_shell_v1_layer
  // 0: background, 1: button, 2: top, 3: overlay
  struct zn_snode *layers[4];  // each snode is @nonnull, @outlive

  vec2 size;  // effective coordinate

  vec2 layout_position;  // layout coordinate

  struct {
    struct wl_signal resized;                  // (NULL)
    struct wl_signal destroy;                  // (NULL)
    struct wl_signal layout_position_changed;  // (NULL)
  } events;
};

void zn_screen_set_layout_position(
    struct zn_screen *self, vec2 layout_position);

/// @param fbox : Effective coordinate system
void zn_screen_damage(struct zn_screen *self, struct wlr_fbox *fbox);

/// Called by the impl object
/// @param size : effective coords
void zn_screen_notify_resize(struct zn_screen *self, vec2 size);

/// Called by the impl object
void zn_screen_notify_frame(struct zn_screen *self, struct timespec *when);

/// Called by the impl object
struct zn_screen *zn_screen_create(
    void *impl_data, const struct zn_screen_interface *implementation);

/// Called by the impl object
void zn_screen_destroy(struct zn_screen *self);
