#pragma once

#include "zen-desktop/cursor-grab.h"

struct zn_cursor_down_grab {
  struct zn_cursor_grab base;
};

bool zn_cursor_down_grab_start(void);
