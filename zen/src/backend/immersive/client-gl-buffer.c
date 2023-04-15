#include "client-gl-buffer.h"

#include <zwin-gl-protocol.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/backend.h"
#include "zen/gl-buffer.h"
#include "zen/server.h"
#include "zen/xr-dispatcher.h"
#include "zen/xr-system.h"

static void zn_client_gl_buffer_destroy(struct zn_client_gl_buffer *self);

static void
zn_client_gl_buffer_handle_destroy(struct wl_resource *resource)
{
  struct zn_client_gl_buffer *self = wl_resource_get_user_data(resource);

  zn_client_gl_buffer_destroy(self);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_buffer_protocol_destroy(
    struct wl_client *client UNUSED, struct wl_resource *resource)
{
  wl_resource_destroy(resource);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_buffer_protocol_data(struct wl_client *client UNUSED,
    struct wl_resource *resource UNUSED, uint32_t target UNUSED,
    struct wl_resource *data UNUSED, uint32_t usage UNUSED)
{}

static const struct zwn_gl_buffer_interface implementation = {
    .destroy = zn_client_gl_buffer_protocol_destroy,
    .data = zn_client_gl_buffer_protocol_data,
};

static void
zn_client_gl_buffer_handle_zn_gl_buffer_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_client_gl_buffer *self =
      zn_container_of(listener, self, zn_gl_buffer_destroy_listener);

  zn_client_gl_buffer_destroy(self);
}

struct zn_client_gl_buffer *
zn_client_gl_buffer_create(struct wl_client *client, uint32_t id)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_xr_system *xr_system = zn_backend_get_xr_system(server->backend);

  if (xr_system == NULL || !zn_xr_system_is_connected(xr_system)) {
    zn_error("Failed to get xr_system");
    goto err;
  }

  struct zn_client_gl_buffer *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zn_gl_buffer =
      zn_xr_dispatcher_get_new_gl_buffer(xr_system->default_dispatcher);
  if (self->zn_gl_buffer == NULL) {
    zn_error("Failed to get new zn_gl_buffer");
    goto err_free;
  }

  self->resource = wl_resource_create(client, &zwn_gl_buffer_interface, 1, id);
  if (self->resource == NULL) {
    zn_error("Failed to create a wl_resource");
    goto err_gl_buffer;
  }

  wl_resource_set_implementation(self->resource, &implementation, self,
      zn_client_gl_buffer_handle_destroy);

  self->zn_gl_buffer_destroy_listener.notify =
      zn_client_gl_buffer_handle_zn_gl_buffer_destroy;
  wl_signal_add(&self->zn_gl_buffer->events.destroy,
      &self->zn_gl_buffer_destroy_listener);

  return self;

err_gl_buffer:
  zn_xr_dispatcher_destroy_gl_buffer(
      xr_system->default_dispatcher, self->zn_gl_buffer);

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_client_gl_buffer_destroy(struct zn_client_gl_buffer *self)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_xr_system *xr_system = zn_backend_get_xr_system(server->backend);

  wl_resource_set_implementation(self->resource, &implementation, NULL, NULL);
  wl_list_remove(&self->zn_gl_buffer_destroy_listener.link);

  if (xr_system != NULL && zn_xr_system_is_connected(xr_system)) {
    zn_xr_dispatcher_destroy_gl_buffer(
        xr_system->default_dispatcher, self->zn_gl_buffer);
  }

  free(self);
}
