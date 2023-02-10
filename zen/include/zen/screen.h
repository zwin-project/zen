#pragma once

#include <wayland-server-core.h>

struct zn_screen {
  void *impl;
};

struct zn_screen *zn_screen_create(void *impl);

void zn_screen_destroy(struct zn_screen *self);
