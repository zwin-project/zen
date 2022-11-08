#include "gl-base-technique.h"

#include <zen-common.h>
#include <zigen-gles-v32-protocol.h>

#include "gl-vertex-array.h"

static void zgnr_gl_base_technique_destroy(
    struct zgnr_gl_base_technique_impl *self);
static void zgnr_gl_base_technique_inert(
    struct zgnr_gl_base_technique_impl *self);

static void
zgnr_gl_base_technique_handle_destroy(struct wl_resource *resource)
{
  struct zgnr_gl_base_technique_impl *self =
      wl_resource_get_user_data(resource);

  zgnr_gl_base_technique_destroy(self);
}

/**
 * @param vertex_array is nullable
 */
static void
zgnr_gl_base_technique_set_current_vertex_array(
    struct zgnr_gl_base_technique_impl *self,
    struct zgnr_gl_vertex_array *vertex_array)
{
  if (self->base.current.vertex_array) {
    wl_list_remove(&self->current_vertex_array_destroy_listener.link);
    wl_list_init(&self->current_vertex_array_destroy_listener.link);
  }

  if (vertex_array) {
    wl_signal_add(&vertex_array->events.destroy,
        &self->current_vertex_array_destroy_listener);
  }

  self->base.current.vertex_array = vertex_array;
}

static void
zgnr_gl_base_technique_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zgnr_gl_base_technique_protocol_bind_program(struct wl_client *client,
    struct wl_resource *resource, struct wl_resource *program)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(program);
}

static void
zgnr_gl_base_technique_protocol_bind_vertex_array(struct wl_client *client,
    struct wl_resource *resource, struct wl_resource *vertex_array)
{
  UNUSED(client);
  struct zgnr_gl_base_technique_impl *self =
      wl_resource_get_user_data(resource);
  if (self == NULL) return;

  zgnr_weak_resource_link(&self->pending.vertex_array, vertex_array);
}

static void
zgnr_gl_base_technique_protocol_bind_texture(struct wl_client *client,
    struct wl_resource *resource, uint32_t location,
    struct wl_resource *texture)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(location);
  UNUSED(texture);
}

static void
zgnr_gl_base_technique_protocol_uniform_vector(struct wl_client *client,
    struct wl_resource *resource, uint32_t location, const char *name,
    uint32_t type, uint32_t size, uint32_t count, struct wl_array *value)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(location);
  UNUSED(name);
  UNUSED(type);
  UNUSED(size);
  UNUSED(count);
  UNUSED(value);
}

static void
zgnr_gl_base_technique_protocol_uniform_matrix(struct wl_client *client,
    struct wl_resource *resource, uint32_t location, const char *name,
    uint32_t type, uint32_t col, uint32_t row, uint32_t count,
    uint32_t transpose, struct wl_array *value)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(location);
  UNUSED(name);
  UNUSED(type);
  UNUSED(col);
  UNUSED(row);
  UNUSED(count);
  UNUSED(transpose);
  UNUSED(value);
}

static void
zgnr_gl_base_technique_protocol_draw_arrays(struct wl_client *client,
    struct wl_resource *resource, uint32_t mode, int32_t first, uint32_t count)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(mode);
  UNUSED(first);
  UNUSED(count);
}

static void
zgnr_gl_base_technique_protocol_draw_elements(struct wl_client *client,
    struct wl_resource *resource, uint32_t mode, uint32_t count, uint32_t type,
    struct wl_array *offset, struct wl_resource *element_array_buffer)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(mode);
  UNUSED(count);
  UNUSED(type);
  UNUSED(offset);
  UNUSED(element_array_buffer);
}

/** Be careful, resource can be inert. */
static const struct zgn_gl_base_technique_interface implementation = {
    .destroy = zgnr_gl_base_technique_protocol_destroy,
    .bind_program = zgnr_gl_base_technique_protocol_bind_program,
    .bind_vertex_array = zgnr_gl_base_technique_protocol_bind_vertex_array,
    .bind_texture = zgnr_gl_base_technique_protocol_bind_texture,
    .uniform_vector = zgnr_gl_base_technique_protocol_uniform_vector,
    .uniform_matrix = zgnr_gl_base_technique_protocol_uniform_matrix,
    .draw_arrays = zgnr_gl_base_technique_protocol_draw_arrays,
    .draw_elements = zgnr_gl_base_technique_protocol_draw_elements,
};

static void
zgnr_gl_base_technique_handle_current_vertex_array_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zgnr_gl_base_technique_impl *self =
      zn_container_of(listener, self, current_vertex_array_destroy_listener);

  zgnr_gl_base_technique_set_current_vertex_array(self, NULL);
}

static void
zgnr_gl_base_technique_handle_rendering_unit_commit(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zgnr_gl_base_technique_impl *self =
      zn_container_of(listener, self, rendering_unit_commit_listener);
  struct zgnr_rendering_unit_impl *unit =
      zn_container_of(self->base.unit, unit, base);
  struct zgnr_gl_vertex_array_impl *vertex_array;

  self->base.commited = true;

  vertex_array = zgnr_weak_resource_get_user_data(&self->pending.vertex_array);
  if (vertex_array) {
    zgnr_gl_vertex_array_commit(vertex_array);
    zgnr_gl_base_technique_set_current_vertex_array(self, &vertex_array->base);
  } else {
    zgnr_gl_base_technique_set_current_vertex_array(self, NULL);
  }

  zgnr_rendering_unit_set_current_technique(unit, self);
}

static void
zgnr_gl_base_technique_handle_rendering_unit_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zgnr_gl_base_technique_impl *self =
      zn_container_of(listener, self, rendering_unit_destroy_listener);

  zgnr_gl_base_technique_inert(self);
}

static void
zgnr_gl_base_technique_inert(struct zgnr_gl_base_technique_impl *self)
{
  struct wl_resource *resource = self->resource;
  zgnr_gl_base_technique_destroy(self);
  wl_resource_set_user_data(resource, NULL);
  wl_resource_set_destructor(resource, NULL);
}

struct zgnr_gl_base_technique_impl *
zgnr_gl_base_technique_create(struct wl_client *client, uint32_t id,
    struct zgnr_rendering_unit_impl *unit)
{
  struct zgnr_gl_base_technique_impl *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->resource =
      wl_resource_create(client, &zgn_gl_base_technique_interface, 1, id);
  if (self->resource == NULL) {
    zn_error("Failed to create a wl_resource");
    goto err_free;
  }

  wl_resource_set_implementation(self->resource, &implementation, self,
      zgnr_gl_base_technique_handle_destroy);

  self->base.unit = &unit->base;
  self->base.commited = false;
  self->base.current.vertex_array = NULL;

  wl_signal_init(&self->base.events.destroy);
  zgnr_weak_resource_init(&self->pending.vertex_array);

  self->rendering_unit_destroy_listener.notify =
      zgnr_gl_base_technique_handle_rendering_unit_destroy;
  wl_signal_add(
      &unit->base.events.destroy, &self->rendering_unit_destroy_listener);

  self->rendering_unit_commit_listener.notify =
      zgnr_gl_base_technique_handle_rendering_unit_commit;
  wl_signal_add(&unit->events.on_commit, &self->rendering_unit_commit_listener);

  self->current_vertex_array_destroy_listener.notify =
      zgnr_gl_base_technique_handle_current_vertex_array_destroy;
  wl_list_init(&self->current_vertex_array_destroy_listener.link);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zgnr_gl_base_technique_destroy(struct zgnr_gl_base_technique_impl *self)
{
  wl_signal_emit(&self->base.events.destroy, NULL);

  zgnr_weak_resource_unlink(&self->pending.vertex_array);
  wl_list_remove(&self->current_vertex_array_destroy_listener.link);
  wl_list_remove(&self->rendering_unit_commit_listener.link);
  wl_list_remove(&self->rendering_unit_destroy_listener.link);
  wl_list_remove(&self->base.events.destroy.listener_list);
  free(self);
}
