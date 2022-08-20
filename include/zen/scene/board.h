#ifndef ZEN_BOARD_H
#define ZEN_BOARD_H

#include <wayland-server-core.h>

#include "zen/scene/scene.h"
#include "zen/scene/screen.h"

struct zn_board_screen_assigned_event {
  struct zn_screen *prev_screen;
  struct zn_screen *current_screen;
};

struct zn_board {
  double width, height;

  struct zn_scene *scene;

  // List of mapped zn_view in z-order, from bottom to top
  struct wl_list view_list;  // zn_view::link

  struct wl_list link;  // zn_scene::board_list

  /** Be careful with the data consistency. When the `zn_board::screen` is not
   * NULL, `screen_link` should belongs to the screen's board_list. */
  struct wl_list screen_link;  // zn_screen::board_list
  struct zn_screen *screen;    // nullable, screen which this board belongs.

  struct wl_listener screen_destroy_listener;

  struct {
    struct wl_signal
        screen_assigned;  // (struct *zn_board_screen_assigned_event)
  } events;
};

bool zn_board_is_dangling(struct zn_board *self);

void zn_board_assign_to_screen(struct zn_board *self, struct zn_screen *screen);

struct zn_board *zn_board_create(struct zn_scene *scene);

void zn_board_destroy(struct zn_board *self);

#endif  //  ZEN_BOARD_H
