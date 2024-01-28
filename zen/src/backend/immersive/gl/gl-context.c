#include "backend/immersive/gl-context.h"

#include <zwin-gl-protocol.h>

#include "backend/immersive/client-virtual-object.h"
#include "backend/immersive/shm-buffer.h"
#include "client-gl-base-technique.h"
#include "client-gl-buffer.h"
#include "client-gl-program.h"
#include "client-gl-rendering-unit.h"
#include "client-gl-shader.h"
#include "client-gl-texture.h"
#include "client-gl-vertex-array.h"
#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"
#include "zen/xr-system.h"

static struct zn_cl_context *
zn_gl_context_get(struct wl_resource *resource)
{
  return wl_resource_get_user_data(resource);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_gl_context_protocol_destroy(
    struct wl_client *client UNUSED, struct wl_resource *resource)
{
  wl_resource_destroy(resource);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_gl_context_protocol_create_gl_rendering_unit(struct wl_client *client UNUSED,
    struct wl_resource *resource, uint32_t id UNUSED,
    struct wl_resource *virtual_object_resource UNUSED)
{
  if (zn_gl_context_get(resource) == NULL) {
    return;
  }

  struct zn_client_virtual_object *client_virtual_object =
      zn_client_virtual_object_get(virtual_object_resource);

  zn_client_gl_rendering_unit_create(client, id, client_virtual_object);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_gl_context_protocol_create_gl_buffer(
    struct wl_client *client, struct wl_resource *resource UNUSED, uint32_t id)
{
  if (zn_gl_context_get(resource) == NULL) {
    return;
  }

  zn_client_gl_buffer_create(client, id);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_gl_context_protocol_create_gl_shader(struct wl_client *client,
    struct wl_resource *resource UNUSED, uint32_t id,
    struct wl_resource *buffer_resource, uint32_t type)
{
  if (zn_gl_context_get(resource) == NULL) {
    return;
  }

  struct zn_shm_buffer *shm_buffer = zn_shm_buffer_get(buffer_resource);

  zn_client_gl_shader_create(client, id, shm_buffer->zn_buffer, type);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_gl_context_protocol_create_gl_program(
    struct wl_client *client, struct wl_resource *resource UNUSED, uint32_t id)
{
  if (zn_gl_context_get(resource) == NULL) {
    return;
  }

  zn_client_gl_program_create(client, id);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_gl_context_protocol_create_gl_texture(
    struct wl_client *client, struct wl_resource *resource UNUSED, uint32_t id)
{
  if (zn_gl_context_get(resource) == NULL) {
    return;
  }

  zn_client_gl_texture_create(client, id);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_gl_context_protocol_create_gl_sampler(struct wl_client *client UNUSED,
    struct wl_resource *resource UNUSED, uint32_t id UNUSED)
{
  if (zn_gl_context_get(resource) == NULL) {
    return;
  }
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_gl_context_protocol_create_gl_vertex_array(
    struct wl_client *client, struct wl_resource *resource UNUSED, uint32_t id)
{
  if (zn_gl_context_get(resource) == NULL) {
    return;
  }

  zn_client_gl_vertex_array_create(client, id);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_gl_context_protocol_create_gl_base_technique(struct wl_client *client,
    struct wl_resource *resource, uint32_t id,
    struct wl_resource *rendering_unit_resource)
{
  if (zn_gl_context_get(resource) == NULL) {
    return;
  }

  struct zn_client_gl_rendering_unit *rendering_unit =
      zn_client_gl_rendering_unit_get(rendering_unit_resource);

  zn_client_gl_base_technique_create(client, id, rendering_unit);
}

static const struct zwn_gl_context_interface implementation = {
    .destroy = zn_gl_context_protocol_destroy,
    .create_gl_rendering_unit = zn_gl_context_protocol_create_gl_rendering_unit,
    .create_gl_buffer = zn_gl_context_protocol_create_gl_buffer,
    .create_gl_shader = zn_gl_context_protocol_create_gl_shader,
    .create_gl_program = zn_gl_context_protocol_create_gl_program,
    .create_gl_texture = zn_gl_context_protocol_create_gl_texture,
    .create_gl_sampler = zn_gl_context_protocol_create_gl_sampler,
    .create_gl_vertex_array = zn_gl_context_protocol_create_gl_vertex_array,
    .create_gl_base_technique = zn_gl_context_protocol_create_gl_base_technique,
};

static void
zn_gl_context_handle_xr_system_session_state_changed(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_gl_context *self =
      zn_container_of(listener, self, xr_system_session_state_changed_listener);

  if (!zn_xr_system_is_connected(self->xr_system)) {
    zn_gl_context_destroy(self);
  }
}

static void
zn_gl_context_bind(
    struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
  struct zn_gl_context *self = data;

  struct wl_resource *resource =
      wl_resource_create(client, &zwn_gl_context_interface, (int)version, id);
  if (resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(resource, &implementation, self, NULL);

  wl_list_insert(&self->resource_list, wl_resource_get_link(resource));
}

struct zn_gl_context *
zn_gl_context_create(struct wl_display *display, struct zn_xr_system *xr_system)
{
  if (!zn_assert(zn_xr_system_is_connected(xr_system),
          "xr_system must be connected")) {
    goto err;
  }

  struct zn_gl_context *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_list_init(&self->resource_list);
  self->xr_system = xr_system;
  wl_signal_init(&self->events.destroy);
  self->destroying = false;

  self->global = wl_global_create(
      display, &zwn_gl_context_interface, 1, self, zn_gl_context_bind);
  if (self->global == NULL) {
    zn_error("Failed to create zwn_gl_context global");
    goto err_free;
  }

  self->xr_system_session_state_changed_listener.notify =
      zn_gl_context_handle_xr_system_session_state_changed;
  wl_signal_add(&xr_system->events.session_state_changed,
      &self->xr_system_session_state_changed_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_gl_context_destroy(struct zn_gl_context *self UNUSED)
{
  if (self->destroying) {
    return;
  }

  self->destroying = true;

  zn_signal_emit_mutable(&self->events.destroy, NULL);

  struct wl_resource *resource = NULL;
  wl_resource_for_each (resource, &self->resource_list) {
    wl_resource_set_implementation(resource, &implementation, NULL, NULL);
  }

  wl_global_destroy(self->global);
  wl_list_remove(&self->xr_system_session_state_changed_listener.link);
  wl_list_remove(&self->events.destroy.listener_list);
  free(self);
}
