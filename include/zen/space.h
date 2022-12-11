#pragma once

#include <wlr/render/wlr_texture.h>
#include <zgnr/space.h>

struct zn_space {
  struct zgnr_space *zgnr_space;  // nonnull

  struct wl_list link;  // zn_scene::space_list

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  struct wl_listener zgnr_space_destroy_listener;
};

struct zn_space *zn_space_create(struct zgnr_space *zgnr_space);
