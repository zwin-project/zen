#include "zen/cursor.h"

#include <drm/drm_fourcc.h>
#include <linux/input.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/xcursor.h>

#include "zen-common.h"
#include "zen/scene/screen-layout.h"
#include "zen/server.h"

static void
zn_cursor_handle_new_screen(struct wl_listener* listener, void* data)
{
  struct zn_cursor* self = zn_container_of(listener, self, new_screen_signal);
  if (self->screen == NULL) {
    self->screen = data;
    self->x = self->screen->output->wlr_output->width / 2;
    self->y = self->screen->output->wlr_output->height / 2;
  }
}

static void
zn_cursor_handle_destroy_screen(struct wl_listener* listener, void* data)
{
  struct zn_cursor* self = zn_container_of(listener, self, new_screen_signal);
  struct zn_server* server = zn_server_get_singleton();
  struct zn_screen_layout* screen_layout = server->scene->screen_layout;

  if (self->screen != data) {
    return;
  }

  if (wl_list_empty(&screen_layout->screens)) {
    self->screen = NULL;
  } else {
    self->screen =
        zn_container_of(&screen_layout->screens.next, self->screen, link);
  }
}

void
zn_cursor_render(struct zn_cursor* self, struct zn_screen* screen,
    struct wlr_renderer* renderer)
{
  float transform[9] = {
      1, 0, 0,  //
      0, 1, 0,  //
      0, 0, 1   //
  };

  if (self->screen == screen && self->texture) {
    wlr_render_texture(
        renderer, self->texture, transform, self->x, self->y, 1.f);
  }
}

struct zn_cursor*
zn_cursor_create(void)
{
  struct zn_server* server = zn_server_get_singleton();
  struct zn_screen_layout* screen_layout = server->scene->screen_layout;
  struct wlr_xcursor* xcursor;
  struct wlr_xcursor_image* image;
  struct zn_cursor* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->xcursor_manager = wlr_xcursor_manager_create(NULL, 24);
  if (self->xcursor_manager == NULL) {
    zn_error("Failed to create wlr_xcursor_manager");
    goto err_free;
  }
  wlr_xcursor_manager_load(self->xcursor_manager, 1.f);

  xcursor =
      wlr_xcursor_manager_get_xcursor(self->xcursor_manager, "left_ptr", 1.f);
  if (xcursor == NULL) {
    zn_error("Failed to get xcursor");
    goto err_wlr_xcursor_manager;
  }
  image = xcursor->images[0];

  self->texture = wlr_texture_from_pixels(server->renderer, DRM_FORMAT_ARGB8888,
      image->width * 4, image->width, image->height, image->buffer);

  self->screen = NULL;

  self->new_screen_signal.notify = zn_cursor_handle_new_screen;
  wl_signal_add(&screen_layout->events.new_screen, &self->new_screen_signal);

  self->destroy_screen_signal.notify = zn_cursor_handle_destroy_screen;
  wl_signal_add(
      &screen_layout->events.destroy_screen, &self->destroy_screen_signal);

  return self;

err_wlr_xcursor_manager:
  wlr_xcursor_manager_destroy(self->xcursor_manager);

err_free:
  free(self);

err:
  return NULL;
}

void
zn_cursor_destroy(struct zn_cursor* self)
{
  wl_list_remove(&self->new_screen_signal.link);
  wlr_texture_destroy(self->texture);
  wlr_xcursor_manager_destroy(self->xcursor_manager);
  free(self);
}
