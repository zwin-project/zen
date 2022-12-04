#pragma once

#include "zen/cursor.h"

struct zn_default_cursor_grab {
  struct zn_cursor_grab base;
};

struct zn_default_cursor_grab *zn_default_cursor_grab_create(void);

void zn_default_cursor_grab_destroy(struct zn_default_cursor_grab *self);
