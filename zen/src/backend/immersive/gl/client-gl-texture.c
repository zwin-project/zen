#include "client-gl-texture.h"

#include <zwin-gl-protocol.h>

#include "backend/immersive/shm-buffer.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/backend.h"
#include "zen/gl-texture.h"
#include "zen/server.h"
#include "zen/xr-dispatcher.h"
#include "zen/xr-system.h"

static void zn_client_gl_texture_destroy(struct zn_client_gl_texture *self);

static void
zn_client_gl_texture_handle_destroy(struct wl_resource *resource)
{
  struct zn_client_gl_texture *self = zn_client_gl_texture_get(resource);

  zn_client_gl_texture_destroy(self);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_texture_protocol_destroy(
    struct wl_client *client UNUSED, struct wl_resource *resource)
{
  wl_resource_destroy(resource);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_texture_protocol_image_2d(struct wl_client *client UNUSED,
    struct wl_resource *resource, uint32_t target, int32_t level,
    int32_t internal_format, uint32_t width, uint32_t height, int32_t border,
    uint32_t format, uint32_t type, struct wl_resource *data)
{
  struct zn_client_gl_texture *self = zn_client_gl_texture_get(resource);
  struct zn_shm_buffer *shm_buffer = zn_shm_buffer_get(data);

  if (self == NULL || shm_buffer == NULL) {
    return;
  }

  zn_gl_texture_image_2d(self->zn_gl_texture, target, level, internal_format,
      width, height, border, format, type, shm_buffer->zn_buffer);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_texture_protocol_generate_mipmap(struct wl_client *client UNUSED,
    struct wl_resource *resource, uint32_t target)
{
  struct zn_client_gl_texture *self = zn_client_gl_texture_get(resource);

  if (self == NULL) {
    return;
  }

  zn_gl_texture_generate_mipmap(self->zn_gl_texture, target);
}

static const struct zwn_gl_texture_interface implementation = {
    .destroy = zn_client_gl_texture_protocol_destroy,
    .image_2d = zn_client_gl_texture_protocol_image_2d,
    .generate_mipmap = zn_client_gl_texture_protocol_generate_mipmap,
};

struct zn_client_gl_texture *
zn_client_gl_texture_get(struct wl_resource *resource)
{
  return wl_resource_get_user_data(resource);
}

static void
zn_client_gl_texture_handle_zn_gl_texture_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_client_gl_texture *self =
      zn_container_of(listener, self, zn_gl_texture_destroy_listener);

  zn_client_gl_texture_destroy(self);
}

struct zn_client_gl_texture *
zn_client_gl_texture_create(struct wl_client *client, uint32_t id)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_xr_system *xr_system = zn_backend_get_xr_system(server->backend);

  if (xr_system == NULL || !zn_xr_system_is_connected(xr_system)) {
    zn_error("Failed to get xr_system");
    goto err;
  }

  struct zn_client_gl_texture *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    wl_client_post_no_memory(client);
    goto err;
  }

  self->zn_gl_texture =
      zn_xr_dispatcher_get_new_gl_texture(xr_system->default_dispatcher);
  if (self->zn_gl_texture == NULL) {
    zn_error("Failed to get new zn_gl_texture");
    goto err_free;
  }

  self->resource = wl_resource_create(client, &zwn_gl_texture_interface, 1, id);
  if (self->resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    goto err_gl_texture;
  }

  wl_resource_set_implementation(self->resource, &implementation, self,
      zn_client_gl_texture_handle_destroy);

  self->zn_gl_texture_destroy_listener.notify =
      zn_client_gl_texture_handle_zn_gl_texture_destroy;
  wl_signal_add(&self->zn_gl_texture->events.destroy,
      &self->zn_gl_texture_destroy_listener);

  return self;

err_gl_texture:
  zn_xr_dispatcher_destroy_gl_texture(
      xr_system->default_dispatcher, self->zn_gl_texture);

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_client_gl_texture_destroy(struct zn_client_gl_texture *self)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_xr_system *xr_system = zn_backend_get_xr_system(server->backend);

  wl_resource_set_implementation(self->resource, &implementation, NULL, NULL);
  wl_list_remove(&self->zn_gl_texture_destroy_listener.link);

  if (xr_system != NULL && zn_xr_system_is_connected(xr_system)) {
    zn_xr_dispatcher_destroy_gl_texture(
        xr_system->default_dispatcher, self->zn_gl_texture);
  }

  free(self);
}
