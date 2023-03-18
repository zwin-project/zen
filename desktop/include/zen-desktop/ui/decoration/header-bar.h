#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>

struct zn_snode;
struct zn_ui_close_button;

struct zn_ui_header_bar {
  struct zn_snode *snode;  // @nonnull, @owning

  struct wlr_texture *texture;  // @nullable, @owning

  struct zn_ui_close_button *close_button;  // @nonnull, @owning

  vec2 size;
  bool focused;

  struct {
    struct wl_signal pressed;  // (NULL)
  } events;
};

void zn_ui_header_bar_set_size(struct zn_ui_header_bar *self, vec2 size);

void zn_ui_header_bar_set_focus(struct zn_ui_header_bar *self, bool focused);

struct zn_ui_header_bar *zn_ui_header_bar_create(void);

void zn_ui_header_bar_destroy(struct zn_ui_header_bar *self);
