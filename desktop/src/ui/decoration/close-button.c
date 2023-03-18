#include "zen-desktop/ui/decoration/close-button.h"

#include <cglm/vec2.h>
#include <drm_fourcc.h>

#include "cairo.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen-desktop/shell.h"
#include "zen-desktop/theme.h"
#include "zen-desktop/theme/icon.h"
#include "zen/backend.h"
#include "zen/server.h"
#include "zen/snode.h"

static struct wlr_texture *
zn_ui_close_button_get_texture(void *user_data)
{
  struct zn_ui_close_button *self = user_data;
  return self->texture;
}

static const struct zn_snode_interface implementation = {
    .get_texture = zn_ui_close_button_get_texture,
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

static void
zn_ui_close_button_render(struct zn_ui_close_button *self, cairo_t *cr)
{
  struct zn_theme *theme = zn_desktop_shell_get_theme();

  struct wlr_fbox box = {
      .x = 0,
      .y = 0,
      .width = self->size,
      .height = self->size,
  };

  zn_icon_render(&theme->icon.header_bar.close, cr, &box);
}

static void
zn_ui_close_button_update_texture(struct zn_ui_close_button *self)
{
  struct zn_server *server = zn_server_get_singleton();

  cairo_surface_t *surface = cairo_image_surface_create(
      CAIRO_FORMAT_ARGB32, (int)self->size, (int)self->size);
  if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
    zn_abort("Failed to create a cairo surface");
    goto out;
  }

  cairo_t *cr = cairo_create(surface);
  if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
    zn_abort("Failed to create a cairo context");
    goto out_cairo;
  }

  zn_ui_close_button_render(self, cr);

  if (self->texture) {
    zn_snode_damage_whole(self->snode);
    wlr_texture_destroy(self->texture);
  }

  unsigned char *data = cairo_image_surface_get_data(surface);
  int stride = cairo_image_surface_get_stride(surface);
  int width = cairo_image_surface_get_width(surface);
  int height = cairo_image_surface_get_height(surface);
  self->texture = zn_backend_create_wlr_texture_from_pixels(
      server->backend, DRM_FORMAT_ARGB8888, stride, width, height, data);

  zn_snode_damage_whole(self->snode);

out_cairo:
  cairo_destroy(cr);

out:
  cairo_surface_destroy(surface);
}

void
zn_ui_close_button_set_size(struct zn_ui_close_button *self, float size)
{
  self->size = size;

  zn_ui_close_button_update_texture(self);
}

struct zn_ui_close_button *
zn_ui_close_button_create(void)
{
  struct zn_ui_close_button *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->size = 0.F;
  self->texture = NULL;
  wl_signal_init(&self->events.clicked);

  self->snode = zn_snode_create(self, &implementation);
  if (self->snode == NULL) {
    zn_error("Failed to create a snode");
    goto err_free;
  }

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_ui_close_button_destroy(struct zn_ui_close_button *self)
{
  zn_snode_damage_whole(self->snode);

  if (self->texture) {
    wlr_texture_destroy(self->texture);
  }

  wl_list_remove(&self->events.clicked.listener_list);
  zn_snode_destroy(self->snode);
  free(self);
}
