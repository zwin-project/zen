#ifndef ZEN_SCREEN_H
#define ZEN_SCREEN_H

#include <wayland-server-core.h>

#include "zen/output.h"
#include "zen/scene/scene.h"

struct zn_screen {
  struct zn_output *output;  // zn_output owns zn_screen, nonnull

  struct wl_list views;  // zn_view::link;

  struct wl_list link;  // zn_scene::screens;
};

struct zn_screen *zn_screen_create(
    struct zn_scene *scene, struct zn_output *output);

void zn_screen_destroy(struct zn_screen *self);

#endif  //  ZEN_SCREEN_H
