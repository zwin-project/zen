#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>
#include <wlr/render/wlr_texture.h>

struct zn_ui_close_button {
  struct zn_snode *snode;  // @nonnull, @owning

  struct wlr_texture *texture;  // @nullable, @owning

  struct zn_animation *hover_animation;  // @nonnull, @owning

  vec2 cursor_position;

  bool pressing;

  float size;

  struct wl_listener hover_animation_listener;

  struct {
    struct wl_signal clicked;  // (NULL)
  } events;
};

void zn_ui_close_button_set_size(struct zn_ui_close_button *self, float size);

struct zn_ui_close_button *zn_ui_close_button_create(void);

void zn_ui_close_button_destroy(struct zn_ui_close_button *self);
