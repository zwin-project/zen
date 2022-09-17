#pragma once

#include <wayland-server.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <xcb/xproto.h>

#include "zen/scene/scene.h"

struct zn_view;

struct zn_view_impl {
  struct wlr_surface *(*get_wlr_surface)(struct zn_view *view);
  void (*get_geometry)(struct zn_view *view, struct wlr_box *box);
  void (*for_each_popup_surface)(struct zn_view *view,
      wlr_surface_iterator_func_t iterator, void *user_data);  // nullable
  uint32_t (*set_size)(struct zn_view *view, double width, double height);
  void (*set_position)(struct zn_view *view, double x, double y);  // nulable
  void (*set_activated)(struct zn_view *view, bool activate);
  void (*restack)(
      struct zn_view *view, enum xcb_stack_mode_t mode);  // nullable
  void (*close_popups)(struct zn_view *view);             // nullable
};

enum zn_view_type {
  ZN_VIEW_XDG_TOPLEVEL,
  ZN_VIEW_XWAYLAND,
};

struct zn_view {
  double x, y;

  enum zn_view_type type;

  const struct zn_view_impl *impl;

  struct wl_list link;     // zn_board::view_list;
  struct zn_board *board;  // non null, when mapped

  struct {
    bool resizing;
    uint32_t edges;
    uint32_t serial;
    struct wlr_fbox requested_box;
  } resize_status;

  struct {
    struct wl_signal unmap;
    struct wl_signal destroy;
  } events;
};

void zn_view_bring_to_front(struct zn_view *self);

/**
 * @param board must not be NULL except when this view is unmapped with
 * `zn_view_unmap`
 */
void zn_view_move(struct zn_view *self, struct zn_board *new_board,
    double board_x, double board_y);

/**
 * Add the damage of all surfaces associated with the view to the output where
 * the view id displayed.
 */
void zn_view_damage(struct zn_view *self);

void zn_view_damage_whole(struct zn_view *self);

void zn_view_get_surface_fbox(struct zn_view *self, struct wlr_fbox *fbox);

void zn_view_get_window_fbox(struct zn_view *self, struct wlr_fbox *fbox);

bool zn_view_is_mapped(struct zn_view *self);

void zn_view_map_to_scene(struct zn_view *self, struct zn_scene *scene);

void zn_view_unmap(struct zn_view *self);

void zn_view_init(struct zn_view *self, enum zn_view_type type,
    const struct zn_view_impl *impl);

void zn_view_fini(struct zn_view *self);
