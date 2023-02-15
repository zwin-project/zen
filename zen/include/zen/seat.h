#pragma once

#include <wayland-server-core.h>

struct zn_cursor;

struct zn_seat {
  struct zn_cursor *cursor;  // @nonnull, @owning
};
