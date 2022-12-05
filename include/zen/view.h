#pragma once

#include <cglm/types.h>
#include <wlr/types/wlr_surface.h>

#include "zen/appearance/view.h"

struct zn_board;

/** lifetime of given wlr_surface must be longer than zn_view */
struct zn_view {
  struct wlr_surface *surface;  // nonnull

  struct wl_list link;        // zn_scene::view_list
  struct wl_list board_link;  // zn_board::view_list

  struct zn_board *board;  // nullable
  double x, y;

  struct {
    vec3 position;
    vec2 size;
    versor quaternion;
  } geometry;

  struct wl_listener board_destroy_listener;

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  struct zna_view *appearance;
};

void zn_view_get_surface_fbox(struct zn_view *self, struct wlr_fbox *fbox);

void zn_view_move(
    struct zn_view *view, struct zn_board *board, double x, double y);

/** lifetime of given wlr_surface must be longer than zn_view */
struct zn_view *zn_view_create(struct wlr_surface *surface);

void zn_view_destroy(struct zn_view *self);
