#pragma once

#include <wayland-server.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <xcb/xproto.h>

#include "zen/scene/scene.h"

#define VIEW_DECORATION_BORDER 8
#define VIEW_DECORATION_TITLEBAR 27

struct zn_view;

struct zn_view_impl {
  struct wlr_surface *(*get_wlr_surface)(struct zn_view *view);
  struct wlr_surface *(*get_wlr_surface_at)(struct zn_view *view,
      double view_sx, double view_sy, double *surface_x, double *surface_y);
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

enum zn_view_area_type {
  ZN_VIEW_AREA_TYPE_INVALID = 0,
  ZN_VIEW_AREA_TYPE_SURFACE = 1 << 0,
  ZN_VIEW_AREA_TYPE_TITLEBAR = 1 << 1,

  ZN_VIEW_AREA_TYPE_BORDER_TOP = 1 << 2,
  ZN_VIEW_AREA_TYPE_BORDER_BOTTOM = 1 << 3,
  ZN_VIEW_AREA_TYPE_BORDER_LEFT = 1 << 4,
  ZN_VIEW_AREA_TYPE_BORDER_RIGHT = 1 << 5,
};

struct zn_view {
  double x, y;

  enum zn_view_type type;

  const struct zn_view_impl *impl;

  struct wl_list link;     // zn_board::view_list;
  struct zn_board *board;  // non null, when mapped

  bool requested_client_decoration;

  struct wl_listener surface_resized_listener;

  struct {
    bool resizing;
    uint32_t edges;
    uint32_t last_serial;
  } resize_status;

  struct {
    struct wl_signal surface_resized;
    struct wl_signal unmap;
    struct wl_signal destroy;
  } events;
};

/**
 * Add the damage of all surfaces associated with the view to the output where
 * the view id displayed.
 */
void zn_view_damage(struct zn_view *self);

void zn_view_damage_whole(struct zn_view *self);

void zn_view_bring_to_front(struct zn_view *self);

/**
 * @param board must not be NULL except when this view is unmapped with
 * `zn_view_unmap`
 */
void zn_view_move(struct zn_view *self, struct zn_board *new_board,
    double board_x, double board_y);

void zn_view_get_surface_fbox(struct zn_view *self, struct wlr_fbox *fbox);

void zn_view_get_view_fbox(struct zn_view *self, struct wlr_fbox *fbox);

bool zn_view_has_client_decoration(struct zn_view *self);

bool zn_view_is_mapped(struct zn_view *self);

void zn_view_map_to_scene(struct zn_view *self, struct zn_scene *scene);

void zn_view_unmap(struct zn_view *self);

/**
 * @retval bits sum of enum wlr_edges
 * @param type bits sum of enum zn_view_area_type
 */
uint32_t zn_view_convert_area_type_to_wlr_edges(uint32_t type);

void zn_view_init(struct zn_view *self, enum zn_view_type type,
    const struct zn_view_impl *impl);

void zn_view_fini(struct zn_view *self);
