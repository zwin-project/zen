#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>
#include <wlr/util/box.h>

struct zn_screen;
struct zna_board;
struct zn_view;

#define CURSOR_Z_OFFSET_ON_BOARD 0.0005
#define VIEW_MIN_Z_OFFSET_ON_BOARD 0.0001
#define VIEW_Z_OFFSET_GAP 0.000001

struct zn_board {
  struct wl_list link;  // zn_scene::board_list

  struct wl_list screen_link;  // zn_screen::board_list

  // nonnull when there are one or more screens in the scene
  struct zn_screen *screen;

  struct wl_list view_list;  // zn_view::board_link, sorted from back to front

  struct {
    mat4 transform;  // translation and rotation only
    vec2 size;
  } geometry;

  struct zna_board *appearance;

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  struct wl_listener screen_destroy_listener;
};

void zn_board_reorder_view(struct zn_board *self);

bool zn_board_is_dangling(struct zn_board *self);

void zn_board_send_frame_done(struct zn_board *self, struct timespec *when);

void zn_board_move(struct zn_board *self, vec2 size, mat4 transform);

struct wlr_surface *zn_board_get_surface_at(struct zn_board *self, double x,
    double y, double *surface_x, double *surface_y, struct zn_view **view);

void zn_board_get_effective_size(
    struct zn_board *self, double *width, double *height);

void zn_board_box_effective_to_local_geom(
    struct zn_board *self, struct wlr_fbox *effective, struct wlr_fbox *geom);

/**
 * @param screen is nullable only when used from this board's method
 */
void zn_board_set_screen(struct zn_board *self, struct zn_screen *screen);

struct zn_board *zn_board_create(void);

void zn_board_destroy(struct zn_board *self);
