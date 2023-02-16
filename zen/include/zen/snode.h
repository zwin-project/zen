#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>
#include <wlr/render/wlr_texture.h>
#include <wlr/util/box.h>

struct zn_snode_interface {
  /// @return value is nullable
  struct wlr_texture *(*get_texture)(void *user_data);
};

struct zn_screen;

/// screen node
struct zn_snode {
  void *user_data;                        // @nullable, @outlive if exists
  const struct zn_snode_interface *impl;  // @nonnull, @outlive

  struct zn_snode *parent;        // @nullable, @ref
  vec2 position;                  // effective coords, relative to parent
  vec2 cached_absolute_position;  // effective coords, relative to root

  // When the screen is destroyed, the root snode is destroyed and
  // `position_changed` signal handler will set this NULL.
  // For root snode, this is @outlive
  struct zn_screen *screen;  // @nullable, @ref

  struct wl_list child_node_list;  // zn_snode::link, sorted from back to front
  struct wl_list link;             // zn_snode::child_node_list

  struct wl_listener parent_destroy_listener;
  struct wl_listener parent_position_changed_listener;

  struct {
    struct wl_signal position_changed;  // (NULL)
    struct wl_signal destroy;           // (NULL)
  } events;
};

/// @param parent is nullable
/// @param x is in the parent local coords
/// @param y is in the parent local coords
void zn_snode_set_position(
    struct zn_snode *self, struct zn_snode *parent, float x, float y);

/// @return value is nullable
struct wlr_texture *zn_snode_get_texture(struct zn_snode *self);

/// @param fbox returns the box of `self` relative to the root in the effective
/// coordinate system.
void zn_snode_get_fbox(struct zn_snode *self, struct wlr_fbox *fbox);

struct zn_snode *zn_snode_create(
    void *user_data, const struct zn_snode_interface *implementation);

struct zn_snode *zn_snode_create_root(struct zn_screen *screen);

void zn_snode_destroy(struct zn_snode *self);
