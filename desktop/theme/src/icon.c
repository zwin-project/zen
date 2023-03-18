#include "zen-desktop/theme/icon.h"

#include "zen-common/log.h"
#include "zen-common/util.h"

void
zn_icon_render(struct zn_icon *self, cairo_t *cr, struct wlr_fbox *box)
{
  if (self->svg_handle == NULL) {
    GError *error = NULL;
    self->svg_handle = rsvg_handle_new_from_file(self->svg_path, &error);

    if (self->svg_handle == NULL) {
      zn_warn_once("Failed to load svg icon: %s", error->message);
      g_error_free(error);
      return;
    }
  }

  RsvgRectangle viewport = {
      .x = box->x,
      .y = box->y,
      .width = box->width,
      .height = box->height,
  };

  GError *error = NULL;
  if (!rsvg_handle_render_document(self->svg_handle, cr, &viewport, &error)) {
    zn_warn_once("Failed to render svg: %s", error->message);
    g_error_free(error);
  }
}

void
zn_icon_init(struct zn_icon *self, const char *svg_path)
{
  self->svg_path = svg_path;
  self->svg_handle = NULL;
}

void
zn_icon_fini(struct zn_icon *self UNUSED)
{
  if (self->svg_handle) {
    g_object_unref(self->svg_handle);
  }
}
