#pragma once

#include <wayland-server-core.h>

struct zn_view;
struct zn_snode;
struct zn_ui_decoration;

struct zn_desktop_view {
  struct zn_view *zn_view;  // @nonnull, @outlive

  struct zn_snode *snode;               // @nonnull, @owning
  struct zn_ui_decoration *decoration;  // @nonnull, @owning

  uint32_t resize_edges;  // a bitfield of enum wlr_edges

  struct wl_listener zn_view_unmap_listener;
  struct wl_listener zn_view_move_request_listener;
  struct wl_listener zn_view_resize_request_listener;
  struct wl_listener zn_view_decoration_listener;
  struct wl_listener zn_view_resized_listener;
  struct wl_listener header_pressed_listener;
  struct wl_listener edge_hover_listener;
  struct wl_listener edge_pressed_listener;
  struct wl_listener close_button_clicked_listener;

  struct {
    struct wl_signal destroy;
  } events;
};

struct zn_desktop_view *zn_desktop_view_create(struct zn_view *zn_view);
