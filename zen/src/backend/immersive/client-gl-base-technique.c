#include "client-gl-base-technique.h"

#include <zwin-gl-protocol.h>
#include <zwin-protocol.h>

#include "client-gl-buffer.h"
#include "client-gl-program.h"
#include "client-gl-rendering-unit.h"
#include "client-gl-vertex-array.h"
#include "client-virtual-object.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen-common/wl-array.h"
#include "zen/gl-base-technique.h"
#include "zen/xr-dispatcher.h"

static void zn_client_gl_base_technique_destroy(
    struct zn_client_gl_base_technique *self);

static void
zn_client_gl_base_technique_handle_destroy(struct wl_resource *resource)
{
  struct zn_client_gl_base_technique *self =
      zn_client_gl_base_technique_get(resource);

  zn_client_gl_base_technique_destroy(self);
}

static void
zn_client_gl_base_technique_protocol_destroy(
    struct wl_client *client UNUSED, struct wl_resource *resource)
{
  wl_resource_destroy(resource);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_base_technique_protocol_bind_program(
    struct wl_client *client UNUSED, struct wl_resource *resource,
    struct wl_resource *program_resource)
{
  struct zn_client_gl_base_technique *self =
      zn_client_gl_base_technique_get(resource);

  struct zn_client_gl_program *program =
      zn_client_gl_program_get(program_resource);

  if (self == NULL || program == NULL) {
    return;
  }

  zn_gl_base_technique_bind_program(
      self->zn_gl_base_technique, program->zn_gl_program);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_base_technique_protocol_bind_vertex_array(
    struct wl_client *client UNUSED, struct wl_resource *resource,
    struct wl_resource *vertex_array_resource)
{
  struct zn_client_gl_base_technique *self =
      zn_client_gl_base_technique_get(resource);

  struct zn_client_gl_vertex_array *gl_vertex_array =
      zn_client_gl_vertex_array_get(vertex_array_resource);

  if (self == NULL || gl_vertex_array == NULL) {
    return;
  }

  zn_gl_base_technique_bind_vertex_array(
      self->zn_gl_base_technique, gl_vertex_array->zn_gl_vertex_array);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_base_technique_protocol_bind_texture(
    struct wl_client *client UNUSED, struct wl_resource *resource UNUSED,
    uint32_t binding UNUSED, const char *name UNUSED,
    struct wl_resource *texture UNUSED, uint32_t target UNUSED,
    struct wl_resource *sampler UNUSED)
{}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_base_technique_protocol_uniform_vector(
    struct wl_client *client UNUSED, struct wl_resource *resource,
    uint32_t location, const char *name, uint32_t type, uint32_t size,
    uint32_t count, struct wl_array *value)
{
  struct zn_client_gl_base_technique *self =
      zn_client_gl_base_technique_get(resource);

  if (self == NULL) {
    return;
  }

  size_t expected_size = 0;

  switch ((enum zwn_gl_base_technique_uniform_variable_type)type) {
    case ZWN_GL_BASE_TECHNIQUE_UNIFORM_VARIABLE_TYPE_INT:
      expected_size = sizeof(int32_t) * size * count;
      break;
    case ZWN_GL_BASE_TECHNIQUE_UNIFORM_VARIABLE_TYPE_UINT:
      expected_size = sizeof(uint32_t) * size * count;
      break;
    case ZWN_GL_BASE_TECHNIQUE_UNIFORM_VARIABLE_TYPE_FLOAT:
      expected_size = sizeof(float) * size * count;
      break;
  }

  if (expected_size != value->size) {
    wl_resource_post_error(resource, ZWN_COMPOSITOR_ERROR_INVALID_WL_ARRAY_SIZE,
        "invalid wl_array size");
    return;
  }

  zn_gl_base_technique_uniform_vector(self->zn_gl_base_technique, location,
      name, type, size, count, value->data);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_base_technique_protocol_uniform_matrix(
    struct wl_client *client UNUSED, struct wl_resource *resource UNUSED,
    uint32_t location UNUSED, const char *name UNUSED, uint32_t col UNUSED,
    uint32_t row UNUSED, uint32_t count UNUSED, uint32_t transpose UNUSED,
    struct wl_array *value UNUSED)
{
  struct zn_client_gl_base_technique *self =
      zn_client_gl_base_technique_get(resource);

  if (self == NULL) {
    return;
  }

  size_t expected_size = sizeof(float) * col * row * count;

  if (expected_size != value->size) {
    wl_resource_post_error(resource, ZWN_COMPOSITOR_ERROR_INVALID_WL_ARRAY_SIZE,
        "invalid wl_array size");
    return;
  }

  zn_gl_base_technique_uniform_matrix(self->zn_gl_base_technique, location,
      name, col, row, count, transpose, value->data);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_base_technique_protocol_draw_arrays(
    struct wl_client *client UNUSED, struct wl_resource *resource,
    uint32_t mode, int32_t first, uint32_t count)
{
  struct zn_client_gl_base_technique *self =
      zn_client_gl_base_technique_get(resource);

  if (self == NULL) {
    return;
  }

  zn_gl_base_technique_draw_arrays(
      self->zn_gl_base_technique, mode, first, count);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_gl_base_technique_protocol_draw_elements(
    struct wl_client *client UNUSED, struct wl_resource *resource,
    uint32_t mode, uint32_t count, uint32_t type, struct wl_array *offset_array,
    struct wl_resource *element_array_buffer_resource)
{
  struct zn_client_gl_base_technique *self =
      zn_client_gl_base_technique_get(resource);

  struct zn_client_gl_buffer *element_array_buffer =
      zn_client_gl_buffer_get(element_array_buffer_resource);

  if (self == NULL || element_array_buffer_resource == NULL) {
    return;
  }

  uint64_t offset = 0;

  if (!zn_wl_array_to_uint64_t(offset_array, &offset)) {
    wl_resource_post_error(resource, ZWN_COMPOSITOR_ERROR_INVALID_WL_ARRAY_SIZE,
        "invalid wl_array size");
    return;
  }

  zn_gl_base_technique_draw_elements(self->zn_gl_base_technique, mode, count,
      type, offset, element_array_buffer->zn_gl_buffer);
}

static const struct zwn_gl_base_technique_interface implementation = {
    .destroy = zn_client_gl_base_technique_protocol_destroy,
    .bind_program = zn_client_gl_base_technique_protocol_bind_program,
    .bind_vertex_array = zn_client_gl_base_technique_protocol_bind_vertex_array,
    .bind_texture = zn_client_gl_base_technique_protocol_bind_texture,
    .uniform_vector = zn_client_gl_base_technique_protocol_uniform_vector,
    .uniform_matrix = zn_client_gl_base_technique_protocol_uniform_matrix,
    .draw_arrays = zn_client_gl_base_technique_protocol_draw_arrays,
    .draw_elements = zn_client_gl_base_technique_protocol_draw_elements,
};

struct zn_client_gl_base_technique *
zn_client_gl_base_technique_get(struct wl_resource *resource)
{
  return wl_resource_get_user_data(resource);
}

static void
zn_client_gl_base_technique_handle_rendering_unit_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_client_gl_base_technique *self =
      zn_container_of(listener, self, rendering_unit_destroy_listener);

  zn_client_gl_base_technique_destroy(self);
}

static void
zn_client_gl_base_technique_handle_zn_gl_base_technique_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_client_gl_base_technique *self =
      zn_container_of(listener, self, zn_gl_base_technique_destroy_listener);

  zn_client_gl_base_technique_destroy(self);
}

struct zn_client_gl_base_technique *
zn_client_gl_base_technique_create(struct wl_client *client, uint32_t id,
    struct zn_client_gl_rendering_unit *rendering_unit)
{
  struct zn_client_gl_base_technique *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    wl_client_post_no_memory(client);
    goto err;
  }

  self->rendering_unit = rendering_unit;

  self->zn_gl_base_technique = zn_xr_dispatcher_get_new_gl_base_technique(
      rendering_unit->virtual_object->dispatcher,
      rendering_unit->zn_gl_rendering_unit);
  if (self->zn_gl_base_technique == NULL) {
    zn_error("Failed to get new zn_gl_base_technique");
    wl_client_post_no_memory(client);
    goto err_free;
  }

  self->resource =
      wl_resource_create(client, &zwn_gl_base_technique_interface, 1, id);
  if (self->resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    goto err_gl_base_technique;
  }

  wl_resource_set_implementation(self->resource, &implementation, self,
      zn_client_gl_base_technique_handle_destroy);

  self->rendering_unit_destroy_listener.notify =
      zn_client_gl_base_technique_handle_rendering_unit_destroy;
  wl_signal_add(
      &rendering_unit->events.destroy, &self->rendering_unit_destroy_listener);

  self->zn_gl_base_technique_destroy_listener.notify =
      zn_client_gl_base_technique_handle_zn_gl_base_technique_destroy;
  wl_signal_add(&self->zn_gl_base_technique->events.destroy,
      &self->zn_gl_base_technique_destroy_listener);

  return self;

err_gl_base_technique:
  zn_xr_dispatcher_destroy_gl_base_technique(
      rendering_unit->virtual_object->dispatcher, self->zn_gl_base_technique);

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_client_gl_base_technique_destroy(struct zn_client_gl_base_technique *self)
{
  wl_resource_set_implementation(self->resource, &implementation, NULL, NULL);
  wl_list_remove(&self->rendering_unit_destroy_listener.link);
  wl_list_remove(&self->zn_gl_base_technique_destroy_listener.link);
  zn_xr_dispatcher_destroy_gl_base_technique(
      self->rendering_unit->virtual_object->dispatcher,
      self->zn_gl_base_technique);
  free(self);
}
