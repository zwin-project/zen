#include "gles-v32.h"

#include <zen-common.h>
#include <zigen-gles-v32-protocol.h>

#include "gl-base-technique.h"
#include "gl-buffer.h"
#include "gl-program.h"
#include "gl-shader.h"
#include "gl-texture.h"
#include "gl-vertex-array.h"
#include "rendering-unit.h"
#include "virtual-object.h"
#include "zgnr/shm.h"

static void
zgnr_gles_v32_protocol_destroy(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zgnr_gles_v32_protocol_create_rendering_unit(struct wl_client* client,
    struct wl_resource* resource, uint32_t id,
    struct wl_resource* virtual_object_resource)
{
  struct zgnr_gles_v32_impl* self = wl_resource_get_user_data(resource);
  struct zgnr_rendering_unit_impl* unit;
  struct zgnr_virtual_object_impl* virtual_object =
      wl_resource_get_user_data(virtual_object_resource);

  unit = zgnr_rendering_unit_create(client, id, virtual_object);
  if (unit == NULL) {
    zn_error("Failed to create a rendering unit");
    wl_client_post_no_memory(client);
    return;
  }

  wl_signal_emit(&self->base.events.new_rendering_unit, &unit->base);
}

static void
zgnr_gles_v32_protocol_create_gl_buffer(
    struct wl_client* client, struct wl_resource* resource, uint32_t id)
{
  struct zgnr_gles_v32_impl* self = wl_resource_get_user_data(resource);
  struct zgnr_gl_buffer_impl* buffer = zgnr_gl_buffer_create(client, id);
  if (buffer == NULL) {
    zn_error("Failed to create a gl buffer");
    wl_client_post_no_memory(client);
    return;
  }

  wl_signal_emit(&self->base.events.new_gl_buffer, &buffer->base);
}

static void
zgnr_gles_v32_protocol_create_gl_shader(struct wl_client* client,
    struct wl_resource* resource, uint32_t id,
    struct wl_resource* buffer_resource, uint32_t type)
{
  struct zgnr_gles_v32_impl* self = wl_resource_get_user_data(resource);
  struct zgnr_gl_shader_impl* shader =
      zgnr_gl_shader_create(client, id, buffer_resource, type);
  if (shader == NULL) {
    zn_error("Failed to creat a gl shader");
    wl_client_post_no_memory(client);
    return;
  }

  wl_signal_emit(&self->base.events.new_gl_shader, &shader->base);
}

static void
zgnr_gles_v32_protocol_create_gl_program(
    struct wl_client* client, struct wl_resource* resource, uint32_t id)
{
  struct zgnr_gles_v32_impl* self = wl_resource_get_user_data(resource);

  struct zgnr_gl_program_impl* program = zgnr_gl_program_create(client, id);
  if (program == NULL) {
    zn_error("Failed to creat a gl program");
    wl_client_post_no_memory(client);
    return;
  }

  wl_signal_emit(&self->base.events.new_gl_program, &program->base);
}

static void
zgnr_gles_v32_protocol_create_gl_texture(
    struct wl_client* client, struct wl_resource* resource, uint32_t id)
{
  struct zgnr_gles_v32_impl* self = wl_resource_get_user_data(resource);

  struct zgnr_gl_texture_impl* texture = zgnr_gl_texture_create(client, id);
  if (texture == NULL) {
    zn_error("Failed to creat a gl texture");
    wl_client_post_no_memory(client);
    return;
  }

  wl_signal_emit(&self->base.events.new_gl_texture, &texture->base);
}

static void
zgnr_gles_v32_protocol_create_gl_vertex_array(
    struct wl_client* client, struct wl_resource* resource, uint32_t id)
{
  struct zgnr_gles_v32_impl* self = wl_resource_get_user_data(resource);
  struct zgnr_gl_vertex_array_impl* vertex_array =
      zgnr_gl_vertex_array_create(client, id);
  if (self == NULL) {
    zn_error("Failed to creat a gl vertex array");
    wl_client_post_no_memory(client);
    return;
  }

  wl_signal_emit(&self->base.events.new_gl_vertex_array, &vertex_array->base);
}

static void
zgnr_gles_v32_protocol_create_gl_base_technique(struct wl_client* client,
    struct wl_resource* resource, uint32_t id,
    struct wl_resource* unit_resource)
{
  struct zgnr_gles_v32_impl* self = wl_resource_get_user_data(resource);
  struct zgnr_rendering_unit_impl* unit =
      wl_resource_get_user_data(unit_resource);
  struct zgnr_gl_base_technique_impl* technique;

  technique = zgnr_gl_base_technique_create(client, id, unit);
  if (technique == NULL) {
    zn_error("Failed to create a gl base technique");
    wl_client_post_no_memory(client);
    return;
  }

  wl_signal_emit(&self->base.events.new_gl_base_technique, &technique->base);
}

static const struct zgn_gles_v32_interface implementation = {
    .destroy = zgnr_gles_v32_protocol_destroy,
    .create_rendering_unit = zgnr_gles_v32_protocol_create_rendering_unit,
    .create_gl_buffer = zgnr_gles_v32_protocol_create_gl_buffer,
    .create_gl_shader = zgnr_gles_v32_protocol_create_gl_shader,
    .create_gl_program = zgnr_gles_v32_protocol_create_gl_program,
    .create_gl_texture = zgnr_gles_v32_protocol_create_gl_texture,
    .create_gl_vertex_array = zgnr_gles_v32_protocol_create_gl_vertex_array,
    .create_gl_base_technique = zgnr_gles_v32_protocol_create_gl_base_technique,
};

static void
zgnr_gles_v32_bind(
    struct wl_client* client, void* data, uint32_t version, uint32_t id)
{
  struct zgnr_gles_v32_impl* self = data;

  struct wl_resource* resource =
      wl_resource_create(client, &zgn_gles_v32_interface, version, id);
  if (resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(resource, &implementation, self, NULL);
}

struct zgnr_gles_v32*
zgnr_gles_v32_create(struct wl_display* display)
{
  struct zgnr_gles_v32_impl* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_signal_init(&self->base.events.new_gl_base_technique);
  wl_signal_init(&self->base.events.new_gl_buffer);
  wl_signal_init(&self->base.events.new_gl_program);
  wl_signal_init(&self->base.events.new_gl_shader);
  wl_signal_init(&self->base.events.new_gl_texture);
  wl_signal_init(&self->base.events.new_gl_vertex_array);
  wl_signal_init(&self->base.events.new_rendering_unit);
  self->display = display;

  self->global = wl_global_create(
      self->display, &zgn_gles_v32_interface, 1, self, zgnr_gles_v32_bind);
  if (self->global == NULL) {
    zn_error("Failed to create wl_global");
    goto err_free;
  }

  return &self->base;

err_free:
  free(self);

err:
  return NULL;
}

void
zgnr_gles_v32_destroy(struct zgnr_gles_v32* parent)
{
  struct zgnr_gles_v32_impl* self = zn_container_of(parent, self, base);

  wl_global_destroy(self->global);

  wl_list_remove(&self->base.events.new_rendering_unit.listener_list);
  wl_list_remove(&self->base.events.new_gl_vertex_array.listener_list);
  wl_list_remove(&self->base.events.new_gl_texture.listener_list);
  wl_list_remove(&self->base.events.new_gl_shader.listener_list);
  wl_list_remove(&self->base.events.new_gl_program.listener_list);
  wl_list_remove(&self->base.events.new_gl_buffer.listener_list);
  wl_list_remove(&self->base.events.new_gl_base_technique.listener_list);

  free(self);
}
