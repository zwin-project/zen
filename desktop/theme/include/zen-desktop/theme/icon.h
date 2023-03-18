#pragma once

#include <cairo.h>
#include <librsvg/rsvg.h>
#include <wlr/util/box.h>

struct zn_icon {
  const char *svg_path;  // @nonnull, @outlive

  RsvgHandle *svg_handle;  // @nullable, @owning
};

void zn_icon_render(struct zn_icon *self, cairo_t *cr, struct wlr_fbox *box);

/// @param svg_path is nonnull
void zn_icon_init(struct zn_icon *self, const char *svg_path);

void zn_icon_fini(struct zn_icon *self);
