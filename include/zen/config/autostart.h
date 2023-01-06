#pragma once

#include <wayland-server-core.h>

struct zn_autostart {
  char *command;
  struct wl_list link;
};

void zn_autostart_exec(struct zn_autostart *self);

struct zn_autostart *zn_autostart_create(char *command);

void zn_autostart_destroy(struct zn_autostart *self);
