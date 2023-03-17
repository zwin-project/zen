#include "zen-desktop/ui/decoration/shadow.h"

#include <drm_fourcc.h>

#include "cairo.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen-desktop/theme.h"
#include "zen/backend.h"
#include "zen/server.h"
#include "zen/snode.h"

static struct wlr_texture *
zn_ui_decoration_shadow_get_texture(void *user_data)
{
  struct zn_ui_decoration_shadow *self = user_data;
  return self->texture;
}

static const struct zn_snode_interface implementation = {
    .get_texture = zn_ui_decoration_shadow_get_texture,
    .frame = zn_snode_noop_frame,
    .accepts_input = zn_snode_noop_accepts_input,
    .pointer_button = zn_snode_noop_pointer_button,
    .pointer_enter = zn_snode_noop_pointer_enter,
    .pointer_motion = zn_snode_noop_pointer_motion,
    .pointer_leave = zn_snode_noop_pointer_leave,
    .pointer_axis = zn_snode_noop_pointer_axis,
    .pointer_frame = zn_snode_noop_pointer_frame,
    .on_focus = zn_snode_noop_on_focus,
};

void
zn_ui_decoration_shadow_set_size(
    struct zn_ui_decoration_shadow *self, vec2 size)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_theme *theme = zn_theme_get();

  vec2 outer_box = {size[0] + (float)(2 * theme->shadow_blur),
      size[1] + (float)(2 * theme->shadow_blur)};

  cairo_surface_t *surface = cairo_image_surface_create(
      CAIRO_FORMAT_ARGB32, (int)outer_box[0], (int)outer_box[1]);
  if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
    zn_error("Failed to create a cairo surface");
    goto out;
  }

  cairo_t *cr = cairo_create(surface);
  if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
    zn_error("Failed to create a cairo context");
    goto out_cairo;
  }

  struct wlr_fbox box = {
      .x = theme->shadow_blur,
      .y = theme->shadow_blur,
      .width = size[0],
      .height = size[1],
  };

  zn_theme_cairo_drop_shadow(theme, cr, &box);

  if (self->texture) {
    zn_snode_damage_whole(self->image_snode);
    wlr_texture_destroy(self->texture);
  }

  unsigned char *data = cairo_image_surface_get_data(surface);
  int stride = cairo_image_surface_get_stride(surface);
  int width = cairo_image_surface_get_width(surface);
  int height = cairo_image_surface_get_height(surface);
  self->texture = zn_backend_create_wlr_texture_from_pixels(
      server->backend, DRM_FORMAT_ARGB8888, stride, width, height, data);

out_cairo:
  cairo_destroy(cr);

out:
  cairo_surface_destroy(surface);
}

struct zn_ui_decoration_shadow *
zn_ui_decoration_shadow_create(void)
{
  struct zn_ui_decoration_shadow *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->texture = NULL;

  self->snode = zn_snode_create(self, &zn_snode_noop_implementation);
  if (self->snode == NULL) {
    zn_error("Failed to create a snode");
    goto err_free;
  }

  self->image_snode = zn_snode_create(self, &implementation);
  if (self->image_snode == NULL) {
    zn_error("Failed to create a image snode");
    goto err_snode;
  }

  struct zn_theme *theme = zn_theme_get();
  zn_snode_set_position(self->image_snode, self->snode,
      (vec2){-(float)theme->shadow_blur, -(float)theme->shadow_blur});

  return self;

err_snode:
  zn_snode_destroy(self->snode);

err_free:
  free(self);

err:
  return NULL;
}

void
zn_ui_decoration_shadow_destroy(struct zn_ui_decoration_shadow *self)
{
  zn_snode_damage_whole(self->image_snode);

  if (self->texture) {
    wlr_texture_destroy(self->texture);
  }
  zn_snode_destroy(self->image_snode);
  zn_snode_destroy(self->snode);
  free(self);
}
