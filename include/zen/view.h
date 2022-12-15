#pragma once

#include <cglm/types.h>
#include <wlr/types/wlr_surface.h>

#include "zen/appearance/view.h"

struct zn_view;
struct zn_board;
struct zn_xdg_toplevel;

struct zn_view_interface {
  struct wlr_surface *(*get_wlr_surface_at)(struct zn_view *view,
      double view_sx, double view_sy, double *surface_x, double *surface_y);
  void (*set_activated)(struct zn_view *view, bool activated);
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

  struct {
    vec2 size;
    mat4 transform;  // translation and rotation only
  } geometry;

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  struct wl_listener board_destroy_listener;
  struct wl_listener commit_listener;

  struct zna_view *appearance;
};

void zn_view_get_surface_fbox(struct zn_view *self, struct wlr_fbox *fbox);

void zn_view_move(
    struct zn_view *view, struct zn_board *board, double x, double y);

/** lifetime of given wlr_surface must be longer than zn_view */
struct zn_view *zn_view_create(struct wlr_surface *surface,
    const struct zn_view_interface *impl, void *user_data);

void zn_view_destroy(struct zn_view *self);
