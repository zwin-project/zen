#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>
#include <wlr/util/box.h>

struct zn_screen;
struct zna_board;

#define CURSOR_Z_OFFSET_ON_BOARD 0.0005
#define VIEW_Z_OFFSET_ON_BOARD 0.0001

struct zn_board {
  struct wl_list link;  // zn_scene::board_list

  struct zn_screen *screen;  // nullable

  struct wl_list view_list;  // zn_view::board_link

  float color[3];  // FIXME: debugging purpose, remove me later

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

bool zn_board_is_dangling(struct zn_board *self);

void zn_board_send_frame_done(struct zn_board *self, struct timespec *when);

void zn_board_move(struct zn_board *self, vec2 size, mat4 transform);

struct wlr_surface *zn_board_get_surface_at(struct zn_board *self, double x,
    double y, double *surface_x, double *surface_y);

void zn_board_get_effective_size(
    struct zn_board *self, double *width, double *height);

void zn_board_box_effective_to_local_geom(
    struct zn_board *self, struct wlr_fbox *effective, struct wlr_fbox *geom);

/**
 * @param screen is nullable
 */
void zn_board_set_screen(struct zn_board *self, struct zn_screen *screen);

struct zn_board *zn_board_create(void);

void zn_board_destroy(struct zn_board *self);
