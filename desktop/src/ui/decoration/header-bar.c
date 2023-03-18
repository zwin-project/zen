#include "zen-desktop/ui/decoration/header-bar.h"

#include <cairo.h>
#include <cglm/vec2.h>
#include <drm_fourcc.h>
#include <zen-common/log.h>
#include <zen-common/util.h>

#include "zen-desktop/shell.h"
#include "zen-desktop/theme.h"
#include "zen/backend.h"
#include "zen/server.h"
#include "zen/snode.h"

struct wlr_texture *
zn_ui_header_bar_get_texture(void *user_data)
{
  struct zn_ui_header_bar *self = user_data;
  return self->texture;
}

static bool
zn_ui_header_accepts_input(void *user_data, const vec2 point)
{
  struct zn_ui_header_bar *self = user_data;
  return 0 <= point[0] && point[0] <= self->size[0] && 0 <= point[1] &&
         point[1] <= self->size[1];
}

static void
zn_ui_header_pointer_button(void *user_data, uint32_t time_msec UNUSED,
    uint32_t button UNUSED, enum wlr_button_state state)
{
  struct zn_ui_header_bar *self = user_data;

  if (state == WLR_BUTTON_PRESSED) {
    wl_signal_emit(&self->events.pressed, NULL);
  }
}

static const struct zn_snode_interface snode_implementation = {
    .get_texture = zn_ui_header_bar_get_texture,
    .frame = zn_snode_noop_frame,
    .accepts_input = zn_ui_header_accepts_input,
    .pointer_button = zn_ui_header_pointer_button,
    .pointer_enter = zn_snode_noop_pointer_enter,
    .pointer_motion = zn_snode_noop_pointer_motion,
    .pointer_leave = zn_snode_noop_pointer_leave,
    .pointer_axis = zn_snode_noop_pointer_axis,
    .pointer_frame = zn_snode_noop_pointer_frame,
    .on_focus = zn_snode_noop_on_focus,
};

static void
zn_ui_header_bar_render(struct zn_ui_header_bar *self, cairo_t *cr)
{
  struct zn_theme *theme = zn_desktop_shell_get_theme();

  float r = theme->radius.header_bar.corner;
  float w = self->size[0];
  float h = self->size[1];
  float reflection_offset = 1.5F;

  if (self->focused) {
    zn_color_set_cairo_source(&theme->color.header_bar.active_reflection, cr);
  } else {
    zn_color_set_cairo_source(&theme->color.header_bar.inactive_reflection, cr);
  }

  cairo_move_to(cr, 0, h);
  cairo_line_to(cr, 0, r);
  cairo_arc(cr, r, r, r, -M_PI, -M_PI_2);
  cairo_line_to(cr, w - r, 0);
  cairo_arc(cr, w - r, r, r, -M_PI_2, 0);
  cairo_line_to(cr, w, h);
  cairo_line_to(cr, 0, h);

  cairo_fill(cr);

  if (self->focused) {
    zn_color_set_cairo_source(&theme->color.header_bar.active, cr);
  } else {
    zn_color_set_cairo_source(&theme->color.header_bar.inactive, cr);
  }

  cairo_move_to(cr, 0, h);
  cairo_line_to(cr, 0, r + reflection_offset);
  cairo_arc(cr, r, r + reflection_offset, r, -M_PI, -M_PI_2);
  cairo_line_to(cr, w - r, reflection_offset);
  cairo_arc(cr, w - r, r + reflection_offset, r, -M_PI_2, 0);
  cairo_line_to(cr, w, h);
  cairo_line_to(cr, 0, h);

  cairo_fill(cr);
}

static void
zn_ui_header_bar_update_texture(struct zn_ui_header_bar *self)
{
  struct zn_server *server = zn_server_get_singleton();

  cairo_surface_t *surface = cairo_image_surface_create(
      CAIRO_FORMAT_ARGB32, (int)self->size[0], (int)self->size[1]);
  if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
    zn_abort("Failed to create a cairo surface");
    goto out;
  }

  cairo_t *cr = cairo_create(surface);
  if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
    zn_abort("Failed to create a cairo context");
    goto out_cairo;
  }

  zn_ui_header_bar_render(self, cr);

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
zn_ui_header_bar_set_size(struct zn_ui_header_bar *self, vec2 size)
{
  glm_vec2_copy(size, self->size);

  zn_ui_header_bar_update_texture(self);
}

void
zn_ui_header_bar_set_focus(struct zn_ui_header_bar *self, bool focused)
{
  self->focused = focused;

  zn_ui_header_bar_update_texture(self);
}

struct zn_ui_header_bar *
zn_ui_header_bar_create(void)
{
  struct zn_ui_header_bar *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->snode = zn_snode_create(self, &snode_implementation);
  if (self->snode == NULL) {
    zn_error("Failed to create a snode");
    goto err_free;
  }

  glm_vec2_copy(GLM_VEC2_ZERO, self->size);
  wl_signal_init(&self->events.pressed);

  self->texture = NULL;
  self->focused = false;

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_ui_header_bar_destroy(struct zn_ui_header_bar *self)
{
  zn_snode_damage_whole(self->snode);

  if (self->texture) {
    wlr_texture_destroy(self->texture);
  }
  wl_list_remove(&self->events.pressed.listener_list);
  zn_snode_destroy(self->snode);
  free(self);
}
