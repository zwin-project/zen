#ifndef ZEN_SCREEN_H
#define ZEN_SCREEN_H

#include <wayland-server-core.h>

#include "zen/output.h"
#include "zen/scene/screen-layout.h"

struct zn_screen {
  struct wlr_box box;
  struct zn_output *output;  // zn_output owns zn_screen, nonnull
  struct zn_screen_layout *screen_layout;

  struct wl_list views;  // zn_view::link;

  struct wl_list link;  // zn_screen_layout::screens;

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;
};

struct zn_screen *zn_screen_create(
    struct zn_screen_layout *screen_layout, struct zn_output *output);

void zn_screen_destroy(struct zn_screen *self);

#endif  //  ZEN_SCREEN_H
