#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>

struct zn_snode;

struct zn_ui_header_bar {
  struct zn_snode *snode;  // @nonnull, @owning

  vec2 size;

  struct {
    struct wl_signal move;
  } events;
};

void zn_ui_header_bar_set_size(struct zn_ui_header_bar *self, vec2 size);

struct zn_ui_header_bar *zn_ui_header_bar_create(void);

void zn_ui_header_bar_destroy(struct zn_ui_header_bar *self);
