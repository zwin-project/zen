#ifndef ZEN_SCREEN_H
#define ZEN_SCREEN_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_surface.h>

#include "zen/output.h"
#include "zen/scene/board.h"
#include "zen/scene/screen-layout.h"

typedef void (*zn_screen_for_each_visible_surface_callback_t)(
    struct wlr_surface *surface, void* data);

struct zn_screen {
  int x, y;
  struct zn_output *output;  // zn_output owns zn_screen, nonnull
  struct zn_screen_layout *screen_layout;

  struct wl_list board_list;       // zn_board::screen_link, non empty
  struct zn_board *current_board;  // non null
  struct wl_listener current_board_screen_assigned_listener;

  struct wl_list link;  // zn_screen_layout::screens;

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;
};

void zn_screen_for_each_visible_surface(struct zn_screen *self,
    zn_screen_for_each_visible_surface_callback_t callback, void* data);

struct zn_view *zn_screen_get_view_at(
    struct zn_screen *self, double x, double y, double *view_x, double *view_y);

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
    struct zn_screen *self, int x, int y, int *dst_x, int *dst_y);

void zn_screen_get_box(struct zn_screen *self, struct wlr_box *box);

struct zn_screen *zn_screen_create(
    struct zn_screen_layout *screen_layout, struct zn_output *output);

void zn_screen_destroy(struct zn_screen *self);

#endif  //  ZEN_SCREEN_H
