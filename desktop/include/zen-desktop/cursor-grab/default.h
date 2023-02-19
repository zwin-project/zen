#pragma once

#include "zen-desktop/cursor-grab.h"

struct zn_cursor_default_grab {
  struct zn_cursor_grab base;
};

struct zn_cursor_default_grab *zn_cursor_default_grab_create(void);
