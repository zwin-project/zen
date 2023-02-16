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
  return self->xcursor_texture;
}

static const struct zn_snode_interface snode_implementation = {
    .get_texture = zn_cursor_get_texture,
};

void
zn_cursor_notify_motion(
    struct zn_cursor *self, struct zn_cursor_motion_event *event)
{
  wl_signal_emit(&self->events.motion, event);
}

struct zn_cursor *
zn_cursor_create(void)
{
  struct zn_cursor *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->snode = zn_snode_create(self, &snode_implementation);
  if (self->snode == NULL) {
    zn_error("Failed to create a zn_snode");
    goto err_free;
  }

  self->xcursor_manager = wlr_xcursor_manager_create(NULL, 24);
  if (self->xcursor_manager == NULL) {
    zn_error("Failed to create a wlr_xcursor_manager");
    goto err_snode;
  }

  if (!wlr_xcursor_manager_load(self->xcursor_manager, 1.0F)) {
    zn_error("Failed to load xcursor manager");
    goto err_xcursor_manager;
  }

  self->xcursor_texture = NULL;
  struct wlr_xcursor *xcursor =
      wlr_xcursor_manager_get_xcursor(self->xcursor_manager, "left_ptr", 1.0F);
  if (xcursor) {
    struct zn_server *server = zn_server_get_singleton();
    struct wlr_xcursor_image *image = xcursor->images[0];
    self->xcursor_texture = wlr_texture_from_pixels(
        server->backend->wlr_renderer, DRM_FORMAT_ARGB8888, image->width * 4,
        image->width, image->height, image->buffer);
  }

  wl_signal_init(&self->events.motion);

  return self;

err_xcursor_manager:
  wlr_xcursor_manager_destroy(self->xcursor_manager);

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
  wl_list_remove(&self->events.motion.listener_list);
  if (self->xcursor_texture) {
    wlr_texture_destroy(self->xcursor_texture);
  }
  wlr_xcursor_manager_destroy(self->xcursor_manager);
  zn_snode_destroy(self->snode);
  free(self);
}
