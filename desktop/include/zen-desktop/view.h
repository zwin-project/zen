#pragma once

#include <wayland-server-core.h>

struct zn_view;
struct zn_snode;

struct zn_desktop_view {
  struct zn_view *zn_view;  // @nonnull, @outlive

  struct zn_snode *snode;

  struct wl_listener zn_view_unmap_listener;
  struct wl_listener zn_view_move_listener;

  struct {
    struct wl_signal destroy;
  } events;
};

struct zn_desktop_view *zn_desktop_view_create(struct zn_view *zn_view);
