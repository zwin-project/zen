#include "gl-base-technique.h"

#include <zen-common.h>
#include <zwin-gles-v32-protocol.h>
#include <zwin-protocol.h>

#include "gl-buffer.h"
#include "gl-program.h"
#include "gl-uniform-variable.h"
#include "gl-vertex-array.h"
#include "texture-binding.h"

static void zwnr_gl_base_technique_destroy(
    struct zwnr_gl_base_technique_impl *self);
static void zwnr_gl_base_technique_inert(
    struct zwnr_gl_base_technique_impl *self);

static void
zwnr_gl_base_technique_handle_destroy(struct wl_resource *resource)
{
  struct zwnr_gl_base_technique_impl *self =
      wl_resource_get_user_data(resource);

  zwnr_gl_base_technique_destroy(self);
}

/**
 * @param vertex_array is nullable
 */
static void
zwnr_gl_base_technique_set_current_vertex_array(
    struct zwnr_gl_base_technique_impl *self,
    struct zwnr_gl_vertex_array *vertex_array)
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

/**
 * @param program is nullable
 */
static void
zwnr_gl_base_technique_set_current_program(
    struct zwnr_gl_base_technique_impl *self, struct zwnr_gl_program *program)
{
  if (self->base.current.program) {
    wl_list_remove(&self->current_program_destroy_listener.link);
    wl_list_init(&self->current_program_destroy_listener.link);
  }

  if (program) {
    wl_signal_add(
        &program->events.destroy, &self->current_program_destroy_listener);
  }

  self->base.current.program = program;
}

/**
 * @param element_array_buffer is nullable
 */
static void
zwnr_gl_base_technique_set_current_element_array_buffer(
    struct zwnr_gl_base_technique_impl *self,
    struct zwnr_gl_buffer *element_array_buffer)
{
  if (self->base.current.element_array_buffer) {
    wl_list_remove(&self->current_element_array_buffer_destroy_listener.link);
    wl_list_init(&self->current_element_array_buffer_destroy_listener.link);
  }

  if (element_array_buffer) {
    wl_signal_add(&element_array_buffer->events.destroy,
        &self->current_element_array_buffer_destroy_listener);
  }

  self->base.current.element_array_buffer = element_array_buffer;
}

static void
zwnr_gl_base_technique_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zwnr_gl_base_technique_protocol_bind_program(struct wl_client *client,
    struct wl_resource *resource, struct wl_resource *program)
{
  UNUSED(client);
  struct zwnr_gl_base_technique_impl *self =
      wl_resource_get_user_data(resource);
  if (self == NULL) return;

  zn_weak_resource_link(&self->pending.program, program);
  self->pending.program_changed = true;
}

static void
zwnr_gl_base_technique_protocol_bind_vertex_array(struct wl_client *client,
    struct wl_resource *resource, struct wl_resource *vertex_array)
{
  UNUSED(client);
  struct zwnr_gl_base_technique_impl *self =
      wl_resource_get_user_data(resource);
  if (self == NULL) return;

  zn_weak_resource_link(&self->pending.vertex_array, vertex_array);
  self->pending.vertex_array_changed = true;
}

static void
zwnr_gl_base_technique_protocol_bind_texture(struct wl_client *client,
    struct wl_resource *resource, uint32_t binding, const char *name,
    struct wl_resource *texture_resource, uint32_t target,
    struct wl_resource *sampler_resource)
{
  struct zwnr_texture_binding_impl *texture_binding;
  struct zwnr_gl_texture_impl *texture;
  struct zwnr_gl_sampler_impl *sampler;
  struct zwnr_gl_base_technique_impl *self =
      wl_resource_get_user_data(resource);
  if (self == NULL) return;

  wl_list_for_each (
      texture_binding, &self->pending.texture_binding_list, base.link) {
    if (texture_binding->base.binding == binding) {
      zwnr_texture_binding_destroy(texture_binding);
      break;
    }
  }

  texture = wl_resource_get_user_data(texture_resource);
  sampler = wl_resource_get_user_data(sampler_resource);
  texture_binding =
      zwnr_texture_binding_create(binding, name, texture, target, sampler);
  if (texture_binding == NULL) {
    zn_error("Failed to create a zwnr_texture_binding");
    wl_client_post_no_memory(client);
    return;
  }

  wl_list_insert(
      &self->pending.texture_binding_list, &texture_binding->base.link);

  self->pending.texture_changed = true;
}

static void
zwnr_gl_base_technique_protocol_uniform_vector(struct wl_client *client,
    struct wl_resource *resource, uint32_t location, const char *name,
    uint32_t type, uint32_t size, uint32_t count, struct wl_array *value)
{
  UNUSED(client);
  struct zwnr_gl_base_technique_impl *self =
      wl_resource_get_user_data(resource);
  struct zwnr_gl_uniform_variable *uniform_variable;

  if (self == NULL) return;

  if (!(0 < size && size <= 4)) {
    wl_resource_post_error(resource,
        ZWN_GL_BASE_TECHNIQUE_ERROR_UNIFORM_VARIABLE,
        "size must be between 1 to 4, but got %d", size);
    return;
  }

  size_t expected_size = 4 * size * count;
  if (value->size < expected_size) {
    wl_resource_post_error(resource, ZWN_COMPOSITOR_ERROR_WL_ARRAY_SIZE,
        "value is expected to be larger than %ld, but actually got %ld",
        expected_size, value->size);
    return;
  }

  uniform_variable = zwnr_gl_uniform_variable_create(
      location, name, type, 1, size, count, false, value->data);

  wl_list_insert(
      self->pending.uniform_variable_list.prev, &uniform_variable->link);
}

static void
zwnr_gl_base_technique_protocol_uniform_matrix(struct wl_client *client,
    struct wl_resource *resource, uint32_t location, const char *name,
    uint32_t col, uint32_t row, uint32_t count, uint32_t transpose,
    struct wl_array *value)
{
  UNUSED(client);
  struct zwnr_gl_base_technique_impl *self =
      wl_resource_get_user_data(resource);
  struct zwnr_gl_uniform_variable *uniform_variable;

  if (self == NULL) return;

  if (!(0 < col && col <= 4 && 0 < row && row <= 4)) {
    wl_resource_post_error(resource,
        ZWN_GL_BASE_TECHNIQUE_ERROR_UNIFORM_VARIABLE,
        "invalid matrix size (%d x %d)", col, row);
    return;
  }

  size_t expected_size = 4 * col * row * count;
  if (value->size < expected_size) {
    wl_resource_post_error(resource, ZWN_COMPOSITOR_ERROR_WL_ARRAY_SIZE,
        "value is expected to be larger than %ld, but actually got %ld",
        expected_size, value->size);
    return;
  }

  uniform_variable = zwnr_gl_uniform_variable_create(location, name,
      ZWN_GL_BASE_TECHNIQUE_UNIFORM_VARIABLE_TYPE_FLOAT, col, row, count,
      transpose, value->data);

  wl_list_insert(
      self->pending.uniform_variable_list.prev, &uniform_variable->link);
}

static void
zwnr_gl_base_technique_protocol_draw_arrays(struct wl_client *client,
    struct wl_resource *resource, uint32_t mode, int32_t first, uint32_t count)
{
  UNUSED(client);
  struct zwnr_gl_base_technique_impl *self =
      wl_resource_get_user_data(resource);
  if (self == NULL) return;

  self->pending.draw_method = ZWNR_GL_BASE_TECHNIQUE_DRAW_METHOD_ARRAYS;
  self->pending.args.arrays.mode = mode;
  self->pending.args.arrays.first = first;
  self->pending.args.arrays.count = count;
  zn_weak_resource_unlink(&self->pending.element_array_buffer);
  self->pending.draw_method_changed = true;
}

static void
zwnr_gl_base_technique_protocol_draw_elements(struct wl_client *client,
    struct wl_resource *resource, uint32_t mode, uint32_t count, uint32_t type,
    struct wl_array *offset_array, struct wl_resource *element_array_buffer)
{
  UNUSED(client);
  uint64_t offset;
  struct zwnr_gl_base_technique_impl *self =
      wl_resource_get_user_data(resource);
  if (self == NULL) return;

  if (zn_array_to_uint64_t(offset_array, &offset)) {
    wl_resource_post_error(resource, ZWN_COMPOSITOR_ERROR_WL_ARRAY_SIZE,
        "offset is expected to be uint64, but array size was %ld",
        offset_array->size);
    return;
  }

  self->pending.draw_method = ZWNR_GL_BASE_TECHNIQUE_DRAW_METHOD_ELEMENTS;
  self->pending.args.elements.mode = mode;
  self->pending.args.elements.count = count;
  self->pending.args.elements.type = type;
  self->pending.args.elements.offset = offset;
  zn_weak_resource_link(
      &self->pending.element_array_buffer, element_array_buffer);
  self->pending.draw_method_changed = true;
}

/** Be careful, resource can be inert. */
static const struct zwn_gl_base_technique_interface implementation = {
    .destroy = zwnr_gl_base_technique_protocol_destroy,
    .bind_program = zwnr_gl_base_technique_protocol_bind_program,
    .bind_vertex_array = zwnr_gl_base_technique_protocol_bind_vertex_array,
    .bind_texture = zwnr_gl_base_technique_protocol_bind_texture,
    .uniform_vector = zwnr_gl_base_technique_protocol_uniform_vector,
    .uniform_matrix = zwnr_gl_base_technique_protocol_uniform_matrix,
    .draw_arrays = zwnr_gl_base_technique_protocol_draw_arrays,
    .draw_elements = zwnr_gl_base_technique_protocol_draw_elements,
};

static void
zwnr_gl_base_technique_handle_current_vertex_array_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zwnr_gl_base_technique_impl *self =
      zn_container_of(listener, self, current_vertex_array_destroy_listener);

  zwnr_gl_base_technique_set_current_vertex_array(self, NULL);
}

static void
zwnr_gl_base_technique_handle_current_program_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zwnr_gl_base_technique_impl *self =
      zn_container_of(listener, self, current_program_destroy_listener);

  zwnr_gl_base_technique_set_current_program(self, NULL);
}

static void
zwnr_gl_base_technique_handle_current_element_array_buffer_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zwnr_gl_base_technique_impl *self = zn_container_of(
      listener, self, current_element_array_buffer_destroy_listener);

  zwnr_gl_base_technique_set_current_element_array_buffer(self, NULL);
}

/**
 * Even in the case of updating uniform variable, the uniform variable is
 * inserted at the end. This ensures that even if the same uniform variable is
 * specified by location and by name, the one applied later will be used.
 */
static void
zwnr_gl_base_technique_commit_uniform_variable(
    struct zwnr_gl_base_technique_impl *self,
    struct zwnr_gl_uniform_variable *variable)
{
  struct zwnr_gl_uniform_variable *current_variable, *tmp;

  wl_list_for_each_safe (
      current_variable, tmp, &self->base.current.uniform_variable_list, link) {
    if (zwnr_gl_uniform_variable_compare(variable, current_variable) == 0) {
      zwnr_gl_uniform_variable_destroy(current_variable);
      break;
    }
  }

  wl_list_insert(
      self->base.current.uniform_variable_list.prev, &variable->link);
  variable->newly_comitted = true;
}

static void
zwnr_gl_base_technique_handle_rendering_unit_commit(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zwnr_gl_base_technique_impl *self =
      zn_container_of(listener, self, rendering_unit_commit_listener);
  struct zwnr_rendering_unit_impl *unit =
      zn_container_of(self->base.unit, unit, base);
  struct zwnr_gl_vertex_array_impl *vertex_array;
  struct zwnr_gl_program_impl *program;
  struct zwnr_texture_binding_impl *texture_binding;

  if (!self->base.comitted)
    zwnr_rendering_unit_set_current_technique(unit, self);

  self->base.comitted = true;

  // draw method
  self->base.current.draw_method_changed = self->pending.draw_method_changed;
  if (self->pending.draw_method_changed) {
    self->base.current.args = self->pending.args;
    self->base.current.draw_method = self->pending.draw_method;

    struct zwnr_gl_buffer_impl *element_array_buffer =
        zn_weak_resource_get_user_data(&self->pending.element_array_buffer);
    zwnr_gl_base_technique_set_current_element_array_buffer(
        self, element_array_buffer ? &element_array_buffer->base : NULL);

    self->pending.draw_method_changed = false;
  }

  if (self->base.current.element_array_buffer) {
    struct zwnr_gl_buffer_impl *element_array_buffer = zn_container_of(
        self->base.current.element_array_buffer, element_array_buffer, base);
    zwnr_gl_buffer_commit(element_array_buffer);
  }

  // uniform variables
  struct zwnr_gl_uniform_variable *uniform_variable, *tmp_uniform_variable;
  wl_list_for_each (
      uniform_variable, &self->base.current.uniform_variable_list, link) {
    uniform_variable->newly_comitted = false;
  }

  wl_list_for_each_safe (uniform_variable, tmp_uniform_variable,
      &self->pending.uniform_variable_list, link) {
    wl_list_remove(&uniform_variable->link);
    zwnr_gl_base_technique_commit_uniform_variable(self, uniform_variable);
  }

  // vertex array
  self->base.current.vertex_array_changed = self->pending.vertex_array_changed;
  if (self->pending.vertex_array_changed) {
    vertex_array = zn_weak_resource_get_user_data(&self->pending.vertex_array);
    zwnr_gl_base_technique_set_current_vertex_array(
        self, vertex_array ? &vertex_array->base : NULL);
    self->pending.vertex_array_changed = false;
  }

  if (self->base.current.vertex_array) {
    vertex_array =
        zn_container_of(self->base.current.vertex_array, vertex_array, base);
    zwnr_gl_vertex_array_commit(vertex_array);
  }

  // program
  self->base.current.program_changed = self->pending.program_changed;
  if (self->pending.program_changed) {
    program = zn_weak_resource_get_user_data(&self->pending.program);
    zwnr_gl_base_technique_set_current_program(
        self, program ? &program->base : NULL);
    self->pending.program_changed = false;
  }

  if (self->base.current.program) {
    program = zn_container_of(self->base.current.program, program, base);
    zwnr_gl_program_commit(program);
  }

  // texture
  self->base.current.texture_changed = self->pending.texture_changed;
  if (self->pending.texture_changed) {
    struct zwnr_texture_binding_impl *tmp, *copy;

    wl_list_for_each_safe (texture_binding, tmp,
        &self->base.current.texture_binding_list, base.link) {
      zwnr_texture_binding_destroy(texture_binding);
    }

    wl_list_for_each (
        texture_binding, &self->pending.texture_binding_list, base.link) {
      struct zwnr_gl_texture_impl *texture =
          zn_container_of(texture_binding->base.texture, texture, base);
      struct zwnr_gl_sampler_impl *sampler =
          zn_container_of(texture_binding->base.sampler, sampler, base);

      copy = zwnr_texture_binding_create(texture_binding->base.binding,
          texture_binding->base.name, texture, texture_binding->base.target,
          sampler);

      wl_list_insert(
          &self->base.current.texture_binding_list, &copy->base.link);
    }

    self->pending.texture_changed = false;
  }

  wl_list_for_each (
      texture_binding, &self->base.current.texture_binding_list, base.link) {
    struct zwnr_gl_texture_impl *texture =
        zn_container_of(texture_binding->base.texture, texture, base);
    struct zwnr_gl_sampler_impl *sampler =
        zn_container_of(texture_binding->base.sampler, sampler, base);

    zwnr_gl_texture_commit(texture);
    zwnr_gl_sampler_commit(sampler);
  }
}

static void
zwnr_gl_base_technique_handle_rendering_unit_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zwnr_gl_base_technique_impl *self =
      zn_container_of(listener, self, rendering_unit_destroy_listener);

  zwnr_gl_base_technique_inert(self);
}

static void
zwnr_gl_base_technique_inert(struct zwnr_gl_base_technique_impl *self)
{
  struct wl_resource *resource = self->resource;
  zwnr_gl_base_technique_destroy(self);
  wl_resource_set_user_data(resource, NULL);
  wl_resource_set_destructor(resource, NULL);
}

struct zwnr_gl_base_technique_impl *
zwnr_gl_base_technique_create(struct wl_client *client, uint32_t id,
    struct zwnr_rendering_unit_impl *unit)
{
  struct zwnr_gl_base_technique_impl *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->resource =
      wl_resource_create(client, &zwn_gl_base_technique_interface, 1, id);
  if (self->resource == NULL) {
    zn_error("Failed to create a wl_resource");
    goto err_free;
  }

  wl_resource_set_implementation(self->resource, &implementation, self,
      zwnr_gl_base_technique_handle_destroy);

  self->base.unit = &unit->base;
  self->base.comitted = false;
  self->base.current.draw_method = ZWNR_GL_BASE_TECHNIQUE_DRAW_METHOD_NONE;
  self->base.current.element_array_buffer = NULL;
  self->base.current.draw_method_changed = false;

  self->base.current.vertex_array = NULL;
  self->base.current.vertex_array_changed = false;

  self->base.current.program = NULL;
  self->base.current.program_changed = false;

  wl_list_init(&self->base.current.texture_binding_list);
  self->base.current.texture_changed = false;

  wl_list_init(&self->pending.texture_binding_list);
  self->pending.texture_changed = false;

  wl_signal_init(&self->base.events.destroy);
  wl_list_init(&self->base.current.uniform_variable_list);
  wl_list_init(&self->pending.uniform_variable_list);

  zn_weak_resource_init(&self->pending.vertex_array);
  self->pending.vertex_array_changed = false;

  zn_weak_resource_init(&self->pending.program);
  self->pending.program_changed = false;

  zn_weak_resource_init(&self->pending.element_array_buffer);
  self->pending.draw_method_changed = false;

  self->rendering_unit_destroy_listener.notify =
      zwnr_gl_base_technique_handle_rendering_unit_destroy;
  wl_signal_add(
      &unit->base.events.destroy, &self->rendering_unit_destroy_listener);

  self->rendering_unit_commit_listener.notify =
      zwnr_gl_base_technique_handle_rendering_unit_commit;
  wl_signal_add(&unit->events.on_commit, &self->rendering_unit_commit_listener);

  self->current_vertex_array_destroy_listener.notify =
      zwnr_gl_base_technique_handle_current_vertex_array_destroy;
  wl_list_init(&self->current_vertex_array_destroy_listener.link);

  self->current_program_destroy_listener.notify =
      zwnr_gl_base_technique_handle_current_program_destroy;
  wl_list_init(&self->current_program_destroy_listener.link);

  self->current_element_array_buffer_destroy_listener.notify =
      zwnr_gl_base_technique_handle_current_element_array_buffer_destroy;
  wl_list_init(&self->current_element_array_buffer_destroy_listener.link);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zwnr_gl_base_technique_destroy(struct zwnr_gl_base_technique_impl *self)
{
  struct zwnr_gl_uniform_variable *uniform_variable, *uniform_variabl_tmp;
  struct zwnr_texture_binding_impl *texture_binding, *texture_binding_tmp;
  wl_signal_emit(&self->base.events.destroy, NULL);

  wl_list_for_each_safe (uniform_variable, uniform_variabl_tmp,
      &self->base.current.uniform_variable_list, link) {
    zwnr_gl_uniform_variable_destroy(uniform_variable);
  }

  wl_list_for_each_safe (uniform_variable, uniform_variabl_tmp,
      &self->pending.uniform_variable_list, link) {
    zwnr_gl_uniform_variable_destroy(uniform_variable);
  }

  wl_list_for_each_safe (texture_binding, texture_binding_tmp,
      &self->base.current.texture_binding_list, base.link) {
    zwnr_texture_binding_destroy(texture_binding);
  }

  wl_list_for_each_safe (texture_binding, texture_binding_tmp,
      &self->pending.texture_binding_list, base.link) {
    zwnr_texture_binding_destroy(texture_binding);
  }

  zn_weak_resource_unlink(&self->pending.program);
  zn_weak_resource_unlink(&self->pending.vertex_array);
  zn_weak_resource_unlink(&self->pending.element_array_buffer);
  wl_list_remove(&self->current_program_destroy_listener.link);
  wl_list_remove(&self->current_vertex_array_destroy_listener.link);
  wl_list_remove(&self->current_element_array_buffer_destroy_listener.link);
  wl_list_remove(&self->rendering_unit_commit_listener.link);
  wl_list_remove(&self->rendering_unit_destroy_listener.link);
  wl_list_remove(&self->base.events.destroy.listener_list);
  free(self);
}
