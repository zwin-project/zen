#include "zen-desktop/ui/decoration/close-button.h"

#include <cglm/vec2.h>
#include <drm_fourcc.h>
#include <linux/input.h>

#include "cairo.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen-desktop/animation.h"
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

static void
zn_ui_close_button_frame(void *user_data, const struct timespec *when UNUSED)
{
  struct zn_ui_close_button *self = user_data;

  zn_animation_notify_frame(self->hover_animation);
}

static bool
zn_ui_close_button_accepts_input(void *user_data, const vec2 point)
{
  struct zn_ui_close_button *self = user_data;
  return (0 < point[0] && point[0] < self->size) &&
         (0 < point[1] && point[1] < self->size);
}

static void
zn_ui_close_button_pointer_button(void *user_data, uint32_t time_msec UNUSED,
    uint32_t button, enum wlr_button_state state)
{
  struct zn_ui_close_button *self = user_data;

  if (button != BTN_LEFT) {
    return;
  }

  if (state == WLR_BUTTON_PRESSED) {
    self->pressing = true;
    return;
  }

  if (!self->pressing) {
    return;
  }

  if ((0 < self->cursor_position[0] && self->cursor_position[0] < self->size) &&
      (0 < self->cursor_position[1] && self->cursor_position[1] < self->size)) {
    wl_signal_emit(&self->events.clicked, NULL);
  }

  self->pressing = false;
}

static void
zn_ui_close_button_pointer_enter(void *user_data, const vec2 point)
{
  struct zn_ui_close_button *self = user_data;
  zn_animation_start(self->hover_animation, 1, 100);
  glm_vec2_copy((float *)point, self->cursor_position);
  self->pressing = false;
}

static void
zn_ui_close_button_pointer_motion(
    void *user_data, uint32_t time_msec UNUSED, const vec2 point)
{
  struct zn_ui_close_button *self = user_data;
  glm_vec2_copy((float *)point, self->cursor_position);
}

static void
zn_ui_close_button_pointer_leave(void *user_data)
{
  struct zn_ui_close_button *self = user_data;
  zn_animation_start(self->hover_animation, 0, 100);
  self->pressing = false;
}

static const struct zn_snode_interface implementation = {
    .get_texture = zn_ui_close_button_get_texture,
    .frame = zn_ui_close_button_frame,
    .accepts_input = zn_ui_close_button_accepts_input,
    .pointer_button = zn_ui_close_button_pointer_button,
    .pointer_enter = zn_ui_close_button_pointer_enter,
    .pointer_motion = zn_ui_close_button_pointer_motion,
    .pointer_leave = zn_ui_close_button_pointer_leave,
    .pointer_axis = zn_snode_noop_pointer_axis,
    .pointer_frame = zn_snode_noop_pointer_frame,
    .on_focus = zn_snode_noop_on_focus,
};

static void
zn_ui_close_button_render(struct zn_ui_close_button *self, cairo_t *cr)
{
  struct zn_theme *theme = zn_desktop_shell_get_theme();
  struct zn_color *hover_color = &theme->color.header_bar.close_button.hover;
  float hover_opacity = self->hover_animation->value;

  struct wlr_fbox box = {
      .x = 0,
      .y = 0,
      .width = self->size,
      .height = self->size,
  };

  cairo_set_source_rgba(cr, hover_color->rgba[0], hover_color->rgba[1],
      hover_color->rgba[2], hover_opacity);
  cairo_arc(cr, self->size / 2, self->size / 2, self->size / 2, 0, M_PI * 2);
  cairo_fill(cr);

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

static void
zn_ui_close_button_handle_hover_animation_frame(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_ui_close_button *self =
      zn_container_of(listener, self, hover_animation_listener);

  zn_ui_close_button_update_texture(self);
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
  glm_vec2_zero(self->cursor_position);
  self->pressing = false;

  self->snode = zn_snode_create(self, &implementation);
  if (self->snode == NULL) {
    zn_error("Failed to create a snode");
    goto err_free;
  }

  self->hover_animation = zn_animation_create(0);
  if (self->hover_animation == NULL) {
    zn_error("Failed to create a hove animation");
    goto err_snode;
  }

  self->hover_animation_listener.notify =
      zn_ui_close_button_handle_hover_animation_frame;
  wl_signal_add(
      &self->hover_animation->events.frame, &self->hover_animation_listener);

  return self;

err_snode:
  zn_snode_destroy(self->snode);

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
    self->texture = NULL;
  }

  zn_snode_destroy(self->snode);
  wl_list_remove(&self->events.clicked.listener_list);
  wl_list_remove(&self->hover_animation_listener.link);
  zn_animation_destroy(self->hover_animation);
  free(self);
}
