#include "client-gl-vertex-array.h"

#include <zwin-gl-protocol.h>
#include <zwin-protocol.h>

#include "client-gl-buffer.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen-common/wl-array.h"
#include "zen/backend.h"
#include "zen/gl-vertex-array.h"
#include "zen/server.h"
#include "zen/xr-dispatcher.h"
#include "zen/xr-system.h"

static void zn_client_gl_vertex_array_destroy(
    struct zn_client_gl_vertex_array *self);

static void
zn_client_gl_vertex_array_handle_destroy(struct wl_resource *resource)
{
  struct zn_client_gl_vertex_array *self =
      zn_client_gl_vertex_array_get(resource);

  zn_client_gl_vertex_array_destroy(self);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_vertex_array_protocol_destroy(
    struct wl_client *client UNUSED, struct wl_resource *resource)
{
  wl_resource_destroy(resource);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_vertex_array_protocol_enable_vertex_attrib_array(
    struct wl_client *client UNUSED, struct wl_resource *resource,
    uint32_t index)
{
  struct zn_client_gl_vertex_array *self =
      zn_client_gl_vertex_array_get(resource);

  if (self == NULL) {
    return;
  }

  zn_gl_vertex_array_enable_vertex_attrib_array(
      self->zn_gl_vertex_array, index);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_vertex_array_protocol_disable_vertex_attrib_array(
    struct wl_client *client UNUSED, struct wl_resource *resource,
    uint32_t index)
{
  struct zn_client_gl_vertex_array *self =
      zn_client_gl_vertex_array_get(resource);

  if (self == NULL) {
    return;
  }

  zn_gl_vertex_array_disable_vertex_attrib_array(
      self->zn_gl_vertex_array, index);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_vertex_array_protocol_vertex_attrib_pointer(
    struct wl_client *client UNUSED, struct wl_resource *resource,
    uint32_t index, int32_t size, uint32_t type, uint32_t normalized,
    int32_t stride, struct wl_array *offset_array,
    struct wl_resource *gl_buffer_resource)
{
  struct zn_client_gl_vertex_array *self =
      zn_client_gl_vertex_array_get(resource);
  struct zn_client_gl_buffer *gl_buffer =
      zn_client_gl_buffer_get(gl_buffer_resource);

  if (self == NULL || gl_buffer == NULL) {
    return;
  }

  uint64_t offset = 0;

  if (!zn_wl_array_to_uint64_t(offset_array, &offset)) {
    wl_resource_post_error(resource, ZWN_COMPOSITOR_ERROR_INVALID_WL_ARRAY_SIZE,
        "invalid wl_array size");
    return;
  }

  zn_gl_vertex_array_vertex_attrib_pointer(self->zn_gl_vertex_array, index,
      size, type, normalized, stride, offset, gl_buffer->zn_gl_buffer);
}

static const struct zwn_gl_vertex_array_interface implementation = {
    .destroy = zn_client_gl_vertex_array_protocol_destroy,
    .enable_vertex_attrib_array =
        zn_client_gl_vertex_array_protocol_enable_vertex_attrib_array,
    .disable_vertex_attrib_array =
        zn_client_gl_vertex_array_protocol_disable_vertex_attrib_array,
    .vertex_attrib_pointer =
        zn_client_gl_vertex_array_protocol_vertex_attrib_pointer,
};

struct zn_client_gl_vertex_array *
zn_client_gl_vertex_array_get(struct wl_resource *resource)
{
  return wl_resource_get_user_data(resource);
}

static void
zn_client_gl_vertex_array_handle_zn_gl_vertex_array_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_client_gl_vertex_array *self =
      zn_container_of(listener, self, zn_gl_vertex_array_destroy_listener);

  zn_client_gl_vertex_array_destroy(self);
}

struct zn_client_gl_vertex_array *
zn_client_gl_vertex_array_create(struct wl_client *client, uint32_t id)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_xr_system *xr_system = zn_backend_get_xr_system(server->backend);

  if (xr_system == NULL || !zn_xr_system_is_connected(xr_system)) {
    zn_error("Failed to get xr_system");
    goto err;
  }

  struct zn_client_gl_vertex_array *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    wl_client_post_no_memory(client);
    goto err;
  }

  self->zn_gl_vertex_array =
      zn_xr_dispatcher_get_new_gl_vertex_array(xr_system->default_dispatcher);
  if (self->zn_gl_vertex_array == NULL) {
    zn_error("Failed to get new gl_vertex_array");
    wl_client_post_no_memory(client);
    goto err_free;
  }

  self->resource =
      wl_resource_create(client, &zwn_gl_vertex_array_interface, 1, id);
  if (self->resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    goto err_gl_vertex_array;
  }

  wl_resource_set_implementation(self->resource, &implementation, self,
      zn_client_gl_vertex_array_handle_destroy);

  self->zn_gl_vertex_array_destroy_listener.notify =
      zn_client_gl_vertex_array_handle_zn_gl_vertex_array_destroy;
  wl_signal_add(&self->zn_gl_vertex_array->events.destroy,
      &self->zn_gl_vertex_array_destroy_listener);

  return self;

err_gl_vertex_array:
  zn_xr_dispatcher_destroy_gl_vertex_array(
      xr_system->default_dispatcher, self->zn_gl_vertex_array);

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_client_gl_vertex_array_destroy(struct zn_client_gl_vertex_array *self)
{
  struct zn_server *server = zn_server_get_singleton();
  struct zn_xr_system *xr_system = zn_backend_get_xr_system(server->backend);

  wl_resource_set_implementation(self->resource, &implementation, NULL, NULL);
  wl_list_remove(&self->zn_gl_vertex_array_destroy_listener.link);

  if (xr_system != NULL && zn_xr_system_is_connected(xr_system)) {
    zn_xr_dispatcher_destroy_gl_vertex_array(
        xr_system->default_dispatcher, self->zn_gl_vertex_array);
  }

  free(self);
}
