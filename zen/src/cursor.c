#include "cursor.h"

#include <drm_fourcc.h>
#include <wlr/types/wlr_xcursor_manager.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/backend.h"
#include "zen/server.h"
#include "zen/snode.h"

static struct wlr_texture *
zn_cursor_get_texture(void *user_data)
{
  struct zn_cursor *self = user_data;
  struct wlr_texture *texture = NULL;

  if (self->use_image_surface && self->image_surface) {
    texture = wlr_surface_get_texture(self->image_surface);
  } else {
    texture = self->xcursor_texture;
  }

  return texture;
}

static void
zn_cursor_frame(void *user_data, const struct timespec *when)
{
  struct zn_cursor *self = user_data;
  if (self->image_surface) {
    wlr_surface_send_frame_done(self->image_surface, when);
  }
}

static const struct zn_snode_interface image_snode_implementation = {
    .get_texture = zn_cursor_get_texture,
    .frame = zn_cursor_frame,
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
zn_cursor_set_surface(struct zn_cursor *self, struct wlr_surface *surface,
    int32_t hotspot_x, int32_t hotspot_y, bool force)
{
  if (self->image_surface) {
    wl_list_remove(&self->image_surface_destroy_listener.link);
    wl_list_init(&self->image_surface_destroy_listener.link);
  }

  self->image_surface = surface;

  if (surface) {
    wl_signal_add(
        &surface->events.destroy, &self->image_surface_destroy_listener);
  }

  self->use_image_surface = surface || force;

  if (self->use_image_surface) {
    zn_snode_set_position(self->image_snode, self->snode,
        (vec2){-(float)hotspot_x, -(float)hotspot_y});
  } else {
    zn_cursor_set_xcursor_default(self);
  }
}

bool
zn_cursor_set_xcursor(struct zn_cursor *self, const char *name)
{
  struct zn_server *server = zn_server_get_singleton();
  struct wlr_xcursor *xcursor =
      wlr_xcursor_manager_get_xcursor(self->xcursor_manager, name, 1.0F);

  if (!xcursor) {
    return false;
  }

  struct wlr_xcursor_image *image = xcursor->images[0];
  self->xcursor_texture = zn_backend_create_wlr_texture_from_pixels(
      server->backend, DRM_FORMAT_ARGB8888, image->width * 4, image->width,
      image->height, image->buffer);

  zn_snode_set_position(self->image_snode, self->snode,
      (vec2){-(float)image->hotspot_x, -(float)image->hotspot_y});

  self->use_image_surface = false;

  return true;
}

bool
zn_cursor_set_xcursor_default(struct zn_cursor *self)
{
  return zn_cursor_set_xcursor(self, "left_ptr");
}

static void
zn_cursor_handle_image_surface_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_cursor *self =
      zn_container_of(listener, self, image_surface_destroy_listener);

  zn_cursor_set_surface(self, NULL, 0, 0, false);
}

static void
zn_cursor_handle_server_start(struct wl_listener *listener, void *data UNUSED)
{
  struct zn_cursor *self =
      zn_container_of(listener, self, server_start_listener);

  zn_cursor_set_xcursor_default(self);

  if (self->xcursor_texture == NULL) {
    zn_abort("Failed to create xcursor texture");
  }
}

static void
zn_cursor_handle_server_end(struct wl_listener *listener, void *data UNUSED)
{
  struct zn_cursor *self = zn_container_of(listener, self, server_end_listener);
  if (self->xcursor_texture) {
    wlr_texture_destroy(self->xcursor_texture);
  }
}

struct zn_cursor *
zn_cursor_create(void)
{
  struct zn_server *server = zn_server_get_singleton();

  struct zn_cursor *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->snode = zn_snode_create(self, &zn_snode_noop_implementation);
  if (self->snode == NULL) {
    zn_error("Failed to create a zn_snode");
    goto err_free;
  }

  self->image_snode = zn_snode_create(self, &image_snode_implementation);
  if (self->image_snode == NULL) {
    zn_error("Failed to create a zn_snode");
    goto err_snode;
  }

  self->xcursor_manager = wlr_xcursor_manager_create(NULL, 24);
  if (self->xcursor_manager == NULL) {
    zn_error("Failed to create a wlr_xcursor_manager");
    goto err_image_snode;
  }

  if (!wlr_xcursor_manager_load(self->xcursor_manager, 1.0F)) {
    zn_error("Failed to load xcursor manager");
    goto err_xcursor_manager;
  }

  self->xcursor_texture = NULL;
  self->image_surface = NULL;
  self->use_image_surface = false;

  self->server_start_listener.notify = zn_cursor_handle_server_start;
  wl_signal_add(&server->events.start, &self->server_start_listener);

  self->server_end_listener.notify = zn_cursor_handle_server_end;
  wl_signal_add(&server->events.end, &self->server_end_listener);

  self->image_surface_destroy_listener.notify =
      zn_cursor_handle_image_surface_destroy;
  wl_list_init(&self->image_surface_destroy_listener.link);

  return self;

err_xcursor_manager:
  wlr_xcursor_manager_destroy(self->xcursor_manager);

err_image_snode:
  zn_snode_destroy(self->image_snode);

err_snode:
  zn_snode_destroy(self->snode);

err_free:
  free(self);

err:
  return NULL;
}

void
zn_cursor_destroy(struct zn_cursor *self)
{
  wl_list_remove(&self->server_start_listener.link);
  wl_list_remove(&self->server_end_listener.link);
  wl_list_remove(&self->image_surface_destroy_listener.link);
  wlr_xcursor_manager_destroy(self->xcursor_manager);
  zn_snode_destroy(self->image_snode);
  zn_snode_destroy(self->snode);
  free(self);
}
