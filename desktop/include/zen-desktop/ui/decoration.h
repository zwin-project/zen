#pragma once

#include <cglm/types.h>
#include <stdbool.h>

struct zn_snode;
struct zn_ui_header_bar;
struct zn_ui_decoration_edge;
struct zn_ui_decoration_shadow;

struct zn_ui_decoration {
  struct zn_snode *snode;  // @nonnull, @owning

  vec2 content_size;
  vec2 content_offset;

  struct zn_ui_header_bar *header_bar;     // @nonnull, @owning
  struct zn_ui_decoration_edge *edge;      // @nonnull, @owning
  struct zn_ui_decoration_shadow *shadow;  // @nonnull, @owning
};

/// @param status is a bitfield of enum zn_ui_decoration_status
void zn_ui_decoration_set_focus(struct zn_ui_decoration *self, bool focused);

void zn_ui_decoration_set_content_size(
    struct zn_ui_decoration *self, vec2 size);

struct zn_ui_decoration *zn_ui_decoration_create(void);

void zn_ui_decoration_destroy(struct zn_ui_decoration *self);
