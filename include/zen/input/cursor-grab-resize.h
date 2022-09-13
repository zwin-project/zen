#ifndef ZN_CURSOR_GRAB_RESIZE_H
#define ZN_CURSOR_GRAB_RESIZE_H

#include "zen/cursor.h"
#include "zen/input/cursor-grab.h"
#include "zen/scene/view.h"

struct zn_cursor_grab_resize {
  struct zn_cursor_grab base;
  struct zn_view* view;
  uint32_t edges;

  struct wlr_fbox init_view_box;
  double init_cursor_x, init_cursor_y;
  // distance between cursor's pos and window's pos
  double diff_x, diff_y;

  struct wl_listener view_unmap_listener;
};

void zn_cursor_grab_resize_start(
    struct zn_cursor* cursor, struct zn_view* view, uint32_t edges);

#endif  //  ZN_CURSOR_GRAB_RESIZE_H
