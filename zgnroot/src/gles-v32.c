#include "gles-v32.h"

#include <zen-common.h>
#include <zigen-gles-v32-protocol.h>

#include "rendering-unit.h"
#include "virtual-object.h"

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
  struct zgnr_gles_v32_impl* self = resource->data;
  struct zgnr_rendering_unit_impl* unit;
  struct zgnr_virtual_object_impl* virtual_object =
      virtual_object_resource->data;

  unit = zgnr_rendering_unit_create(client, id, virtual_object);
  if (unit == NULL) {
    zn_error("Failed to create a rendering unit");
    wl_client_post_no_memory(client);
  }

  wl_signal_emit(&self->base.events.new_rendering_unit, unit);
}

static void
zgnr_gles_v32_protocol_create_gl_buffer(
    struct wl_client* client, struct wl_resource* resource, uint32_t id)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(id);
}

static void
zgnr_gles_v32_protocol_create_gl_shader(struct wl_client* client,
    struct wl_resource* resource, uint32_t id, int32_t source, uint32_t size,
    uint32_t type)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(id);
  UNUSED(source);
  UNUSED(size);
  UNUSED(type);
}

static void
zgnr_gles_v32_protocol_create_gl_program(
    struct wl_client* client, struct wl_resource* resource, uint32_t id)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(id);
}

static void
zgnr_gles_v32_protocol_create_gl_texture(struct wl_client* client,
    struct wl_resource* resource, uint32_t id, uint32_t target)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(id);
  UNUSED(target);
}

static void
zgnr_gles_v32_protocol_create_gl_vertex_array(
    struct wl_client* client, struct wl_resource* resource, uint32_t id)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(id);
}

static void
zgnr_gles_v32_protocol_create_gl_base_technique(struct wl_client* client,
    struct wl_resource* resource, uint32_t id, struct wl_resource* unit)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(id);
  UNUSED(unit);
}

static const struct zgn_gles_v32_interface interface = {
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

  wl_resource_set_implementation(resource, &interface, self, NULL);
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

  free(self);
}
