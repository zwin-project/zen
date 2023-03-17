#pragma once

#include <cglm/types.h>

struct zn_snode;

struct zn_ui_decoration_shadow {
  struct zn_snode *snode;        // @nonnull, @owning
  struct zn_snode *image_snode;  // @nonnull, @owning

  struct wlr_texture *texture;  // @nullable, @owning
};

/// @param size is the size of the content area, the shadow attached to its
/// surroundings.
void zn_ui_decoration_shadow_set_size(
    struct zn_ui_decoration_shadow *self, vec2 size);

struct zn_ui_decoration_shadow *zn_ui_decoration_shadow_create(void);

void zn_ui_decoration_shadow_destroy(struct zn_ui_decoration_shadow *self);
