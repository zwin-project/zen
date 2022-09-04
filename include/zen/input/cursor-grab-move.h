#ifndef ZN_CURSOR_GRAB_MOVE_H
#define ZN_CURSOR_GRAB_MOVE_H

#include "zen/cursor.h"
#include "zen/input/cursor-grab.h"
#include "zen/scene/view.h"

struct zn_cursor_grab_move {
  struct zn_cursor_grab base;
  struct zn_view* view;
  struct zn_screen* prev_screen;

  double init_x, init_y;
  double diff_x, diff_y;
};

void zn_cursor_grab_move_start(struct zn_cursor* cursor, struct zn_view* view);

#endif  //  ZN_CURSOR_GRAB_MOVE_H
