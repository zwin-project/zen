#pragma once

#include <cglm/types.h>
#include <wlr/types/wlr_surface.h>

#include "zen/appearance/view.h"

struct zn_view;
struct zn_board;
struct zn_xdg_toplevel;

struct zn_view_interface {
  /** surface_x, surface_y can be NULL */
  struct wlr_surface *(*get_wlr_surface_at)(struct zn_view *view,
      double view_sx, double view_sy, double *surface_x, double *surface_y);
  void (*get_window_geom)(struct zn_view *view, struct wlr_box *box);
  uint32_t (*get_current_configure_serial)(struct zn_view *view);
  uint32_t (*set_size)(struct zn_view *view, double width, double height);
  uint32_t (*set_maximized)(struct zn_view *view, bool maximized);
  void (*set_activated)(struct zn_view *view, bool activated);
  uint32_t (*schedule_configure)(struct zn_view *view);
  void (*close_popups)(struct zn_view *view);
  void (*for_each_popup_surface)(struct zn_view *view,
      wlr_surface_iterator_func_t iterator, void *user_data);
};

/** lifetime of given wlr_surface must be longer than zn_view */
struct zn_view {
  void *user_data;
  struct wlr_surface *surface;  // nonnull

  const struct zn_view_interface *impl;

  struct wl_list link;        // zn_scene::view_list
  struct wl_list board_link;  // zn_board::view_list

  struct zn_board *board;  // nullable
  double x, y;

  // controlled by `zn_view_update_z_index`, called from zn_board
  // 1 is the backmost
  uint32_t z_index;

  // view-local coordinate
  struct wlr_fbox prev_surface_fbox;

  struct {
    bool maximized;
    uint32_t changed_serial;
    struct wlr_fbox reset_box;
  } maximize_status;

  struct {
    bool resizing;
    uint32_t edges;
    uint32_t last_serial;
  } resize_status;

  struct {
    vec2 size;
    mat4 transform;  // translation and rotation only
  } geometry;

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  struct wl_listener board_destroy_listener;
  struct wl_listener commit_listener;

  struct zna_view *_appearance;  // be private
  uint32_t appearance_damage;
};

void zn_view_update_z_index(struct zn_view *self, uint32_t z_index);

void zn_view_commit_appearance(struct zn_view *self);

void zn_view_get_surface_fbox(struct zn_view *self, struct wlr_fbox *fbox);

void zn_view_get_view_fbox(struct zn_view *self, struct wlr_fbox *fbox);

void zn_view_bring_to_front(struct zn_view *self);

void zn_view_move(
    struct zn_view *self, struct zn_board *board, double x, double y);

void zn_view_set_maximized(struct zn_view *self, bool maximized);

/** lifetime of given wlr_surface must be longer than zn_view */
struct zn_view *zn_view_create(struct wlr_surface *surface,
    const struct zn_view_interface *impl, void *user_data);

void zn_view_destroy(struct zn_view *self);
