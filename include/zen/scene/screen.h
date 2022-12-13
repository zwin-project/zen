#pragma once

#include <wayland-server-core.h>
#include <wlr/types/wlr_surface.h>
#include <zigzag.h>

#include "zen/output.h"
#include "zen/scene/board.h"
#include "zen/scene/screen-layout.h"

typedef void (*zn_screen_for_each_visible_surface_callback_t)(
    struct wlr_surface *surface, void *user_data);

struct zn_screen {
  double x, y;
  struct zn_output *output;  // zn_output owns zn_screen, nonnull
  struct zn_screen_layout *screen_layout;

  struct wl_list board_list;       // zn_board::screen_link, non empty
  struct zn_board *current_board;  // non null
  struct wl_listener current_board_screen_assigned_listener;

  struct wl_list link;  // zn_screen_layout::screens;

  struct zigzag_layout *node_layout;

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;
};

void zn_screen_for_each_visible_surface(struct zn_screen *self,
    zn_screen_for_each_visible_surface_callback_t callback, void *data);

/**
 * @param surface_x can be NULL
 * @param surface_y can be NULL
 * @param view can be NULL
 */
struct wlr_surface *zn_screen_get_surface_at(struct zn_screen *self, double x,
    double y, double *surface_x, double *surface_y, struct zn_view **view);

void zn_screen_switch_to_next_board(struct zn_screen *self);

void zn_screen_switch_to_prev_board(struct zn_screen *self);

/**
 * @param board must not be NULL and must be an element of self->board_list
 */
void zn_screen_set_current_board(
    struct zn_screen *self, struct zn_board *board);

/**
 * @return struct zn_board* cannot be NULL
 */
struct zn_board *zn_screen_get_current_board(struct zn_screen *self);

void zn_screen_get_screen_layout_coords(
    struct zn_screen *self, double x, double y, double *dst_x, double *dst_y);

void zn_screen_get_fbox(struct zn_screen *self, struct wlr_fbox *box);

struct zn_screen *zn_screen_create(
    struct zn_screen_layout *screen_layout, struct zn_output *output);

void zn_screen_destroy(struct zn_screen *self);
