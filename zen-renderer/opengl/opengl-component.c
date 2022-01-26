#include "opengl-component.h"

#include <cglm/cglm.h>
#include <string.h>
#include <wayland-server.h>
#include <zen-shell/zen-shell.h>
#include <zigen-opengl-server-protocol.h>

#include "opengl-element-array-buffer.h"
#include "opengl-shader-program.h"
#include "opengl-texture.h"
#include "opengl-vertex-buffer.h"
#include "shader-compiler.h"

WL_EXPORT void zen_opengl_component_destroy(
    struct zen_opengl_component *component);

static void
zen_opengl_component_handle_destroy(struct wl_resource *resource)
{
  struct zen_opengl_component *component;

  component = wl_resource_get_user_data(resource);

  zen_opengl_component_destroy(component);
}

static void
zen_opengl_component_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zen_opengl_component_protocol_attach_vertex_buffer(struct wl_client *client,
    struct wl_resource *resource, struct wl_resource *vertex_buffer)
{
  UNUSED(client);
  struct zen_opengl_component *component;

  component = wl_resource_get_user_data(resource);

  zen_weak_link_set(&component->pending.vertex_buffer_link, vertex_buffer);
}

static void
zen_opengl_component_protocol_attach_element_array_buffer(
    struct wl_client *client, struct wl_resource *resource,
    struct wl_resource *element_array_buffer)
{
  UNUSED(client);
  struct zen_opengl_component *component;

  component = wl_resource_get_user_data(resource);

  zen_weak_link_set(
      &component->pending.element_array_buffer_link, element_array_buffer);
}

static void
zen_opengl_component_protocol_attach_shader_program(struct wl_client *client,
    struct wl_resource *resource, struct wl_resource *shader_program)
{
  UNUSED(client);
  struct zen_opengl_component *component;

  component = wl_resource_get_user_data(resource);

  zen_weak_link_set(&component->pending.shader_program_link, shader_program);
}

static void
zen_opengl_component_protocol_attach_texture(struct wl_client *client,
    struct wl_resource *resource, struct wl_resource *texture)
{
  UNUSED(client);
  struct zen_opengl_component *component;

  component = wl_resource_get_user_data(resource);

  zen_weak_link_set(&component->pending.texture_link, texture);
}

static void
zen_opengl_component_protocol_add_vertex_attribute(struct wl_client *client,
    struct wl_resource *resource, uint32_t index, uint32_t size,
    enum zgn_opengl_vertex_attribute_type type, uint32_t normalized,
    uint32_t stride, uint32_t pointer)
{
  UNUSED(client);
  struct zen_opengl_component *component;
  struct zen_opengl_vertex_attribute *attribute;

  component = wl_resource_get_user_data(resource);

  attribute =
      wl_array_add(&component->pending.vertex_attributes, sizeof(*attribute));
  attribute->index = index;
  attribute->size = size;
  attribute->type = type;
  attribute->normalized = normalized;
  attribute->stride = stride;
  attribute->pointer = pointer;
  component->pending.vertex_attributes_changed = true;
}

static void
zen_opengl_component_protocol_clear_vertex_attribute(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  UNUSED(resource);
  struct zen_opengl_component *component;

  component = wl_resource_get_user_data(resource);

  wl_array_release(&component->pending.vertex_attributes);
  wl_array_init(&component->pending.vertex_attributes);
  component->pending.vertex_attributes_changed = true;
}

static void
zen_opengl_component_protocol_set_topology(struct wl_client *client,
    struct wl_resource *resource, enum zgn_opengl_topology topology)
{
  UNUSED(client);
  UNUSED(resource);
  struct zen_opengl_component *component;

  component = wl_resource_get_user_data(resource);

  component->pending.topology = topology;
  component->pending.topology_changed = true;
}

static void
zen_opengl_component_protocol_set_min(
    struct wl_client *client, struct wl_resource *resource, uint32_t min)
{
  UNUSED(client);
  struct zen_opengl_component *component;

  component = wl_resource_get_user_data(resource);

  component->pending.min = min;
  component->pending.min_changed = true;
}

static void
zen_opengl_component_protocol_set_count(
    struct wl_client *client, struct wl_resource *resource, uint32_t count)
{
  UNUSED(client);
  struct zen_opengl_component *component;

  component = wl_resource_get_user_data(resource);

  component->pending.count = count;
  component->pending.count_changed = true;
}

static const struct zgn_opengl_component_interface opengl_component_interface =
    {
        .destroy = zen_opengl_component_protocol_destroy,
        .attach_vertex_buffer =
            zen_opengl_component_protocol_attach_vertex_buffer,
        .attach_element_array_buffer =
            zen_opengl_component_protocol_attach_element_array_buffer,
        .attach_shader_program =
            zen_opengl_component_protocol_attach_shader_program,
        .attach_texture = zen_opengl_component_protocol_attach_texture,
        .add_vertex_attribute =
            zen_opengl_component_protocol_add_vertex_attribute,
        .clear_vertex_attribute =
            zen_opengl_component_protocol_clear_vertex_attribute,
        .set_topology = zen_opengl_component_protocol_set_topology,
        .set_min = zen_opengl_component_protocol_set_min,
        .set_count = zen_opengl_component_protocol_set_count,
};

static void
zen_opengl_component_update_vao(struct zen_opengl_component *component)
{
  struct zen_opengl_vertex_buffer *vertex_buffer;
  struct zen_opengl_element_array_buffer *element_array_buffer;
  struct zen_opengl_vertex_attribute *attribute;

  vertex_buffer =
      zen_weak_link_get_user_data(&component->current.vertex_buffer_link);
  element_array_buffer = zen_weak_link_get_user_data(
      &component->current.element_array_buffer_link);

  if (vertex_buffer == NULL) return;

  glBindVertexArray(component->vertex_array_id);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer->id);

  if (element_array_buffer)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer->id);

  wl_array_for_each(attribute, &component->current.vertex_attributes)
  {
    glEnableVertexAttribArray(attribute->index);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
    glVertexAttribPointer(attribute->index, attribute->size, attribute->type,
        attribute->normalized, attribute->stride, (void *)attribute->pointer);
#pragma GCC diagnostic pop
  }

  glBindVertexArray(0);
  glDisableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

static void
zen_opengl_component_virtual_object_commit_handler(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zen_opengl_component *component;
  struct zen_opengl_vertex_buffer *vertex_buffer;
  struct zen_opengl_element_array_buffer *element_array_buffer;
  struct zen_opengl_shader_program *shader;
  struct zen_opengl_texture *texture;
  bool update_vao = false;

  component =
      wl_container_of(listener, component, virtual_object_commit_listener);

  vertex_buffer =
      zen_weak_link_get_user_data(&component->pending.vertex_buffer_link);
  if (vertex_buffer) {
    zen_opengl_vertex_buffer_commit(vertex_buffer);
    if (component->current.vertex_buffer_link.resource == NULL)
      update_vao = true;
    zen_weak_link_set(
        &component->current.vertex_buffer_link, vertex_buffer->resource);
  }

  element_array_buffer = zen_weak_link_get_user_data(
      &component->pending.element_array_buffer_link);
  if (element_array_buffer) {
    zen_opengl_element_array_buffer_commit(element_array_buffer);
    if (component->current.element_array_buffer_link.resource == NULL) {
      update_vao = true;
    }
    zen_weak_link_set(&component->current.element_array_buffer_link,
        element_array_buffer->resource);
  }

  shader = zen_weak_link_get_user_data(&component->pending.shader_program_link);
  if (shader) {
    zen_opengl_shader_program_commit(shader);
    zen_weak_link_set(
        &component->current.shader_program_link, shader->resource);
  }

  texture = zen_weak_link_get_user_data(&component->pending.texture_link);
  if (texture) {
    zen_opengl_texture_commit(texture);
    zen_weak_link_set(&component->current.texture_link, texture->resource);
  }

  if (component->pending.min_changed) {
    component->current.min = component->pending.min;
    component->pending.min_changed = false;
  }

  if (component->pending.count_changed) {
    component->current.count = component->pending.count;
    component->pending.count_changed = false;
  }

  if (component->pending.topology_changed) {
    component->current.topology = component->pending.topology;
    component->pending.topology_changed = false;
  }

  if (component->pending.vertex_attributes_changed) {
    wl_array_release(&component->current.vertex_attributes);
    component->current.vertex_attributes = component->pending.vertex_attributes;
    wl_array_init(&component->pending.vertex_attributes);
    component->pending.vertex_attributes_changed = false;
    update_vao = true;
  }

  zen_weak_link_unset(&component->pending.vertex_buffer_link);
  zen_weak_link_unset(&component->pending.element_array_buffer_link);
  zen_weak_link_unset(&component->pending.shader_program_link);
  zen_weak_link_unset(&component->pending.texture_link);

  if (update_vao) zen_opengl_component_update_vao(component);
}

static void
zen_opengl_component_virtual_object_destroy_handler(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zen_opengl_component *component;

  component =
      wl_container_of(listener, component, virtual_object_destroy_listener);

  wl_resource_destroy(component->resource);
}

WL_EXPORT struct zen_opengl_component *
zen_opengl_component_create(struct wl_client *client, uint32_t id,
    struct zen_opengl_renderer *renderer,
    struct zen_virtual_object *virtual_object)
{
  struct zen_opengl_component *component;
  struct wl_resource *resource;

  component = zalloc(sizeof *component);
  if (component == NULL) {
    zen_log("opengl component: failed to allocate memory\n");
    wl_client_post_no_memory(client);
    goto err;
  }

  resource = wl_resource_create(client, &zgn_opengl_component_interface, 1, id);
  if (resource == NULL) {
    zen_log("opengl component: failed to create a resource\n");
    wl_client_post_no_memory(client);
    goto err_resource;
  }

  wl_resource_set_implementation(resource, &opengl_component_interface,
      component, zen_opengl_component_handle_destroy);

  component->resource = resource;
  component->virtual_object = virtual_object;
  wl_list_insert(&renderer->component_list, &component->link);

  glGenVertexArrays(1, &component->vertex_array_id);

  zen_weak_link_init(&component->current.vertex_buffer_link);
  zen_weak_link_init(&component->pending.vertex_buffer_link);
  zen_weak_link_init(&component->current.element_array_buffer_link);
  zen_weak_link_init(&component->pending.element_array_buffer_link);
  zen_weak_link_init(&component->current.shader_program_link);
  zen_weak_link_init(&component->pending.shader_program_link);
  zen_weak_link_init(&component->current.texture_link);
  zen_weak_link_init(&component->pending.texture_link);
  component->current.min = 0;
  component->current.count = 0;
  component->current.topology = ZGN_OPENGL_TOPOLOGY_LINES;
  wl_array_init(&component->current.vertex_attributes);
  component->pending.min_changed = false;
  component->pending.count_changed = false;
  component->pending.topology_changed = false;
  component->pending.vertex_attributes_changed = false;
  wl_array_init(&component->pending.vertex_attributes);

  component->virtual_object_commit_listener.notify =
      zen_opengl_component_virtual_object_commit_handler;
  wl_signal_add(&virtual_object->commit_signal,
      &component->virtual_object_commit_listener);
  component->virtual_object_destroy_listener.notify =
      zen_opengl_component_virtual_object_destroy_handler;
  wl_signal_add(&virtual_object->destroy_signal,
      &component->virtual_object_destroy_listener);

  return component;

err_resource:
  free(component);

err:
  return NULL;
}

WL_EXPORT void
zen_opengl_component_destroy(struct zen_opengl_component *component)
{
  zen_weak_link_unset(&component->current.vertex_buffer_link);
  zen_weak_link_unset(&component->pending.vertex_buffer_link);
  zen_weak_link_unset(&component->current.element_array_buffer_link);
  zen_weak_link_unset(&component->pending.element_array_buffer_link);
  zen_weak_link_unset(&component->current.shader_program_link);
  zen_weak_link_unset(&component->pending.shader_program_link);
  zen_weak_link_unset(&component->current.texture_link);
  zen_weak_link_unset(&component->pending.texture_link);
  wl_array_release(&component->current.vertex_attributes);
  wl_array_release(&component->pending.vertex_attributes);
  wl_list_remove(&component->virtual_object_commit_listener.link);
  wl_list_remove(&component->virtual_object_destroy_listener.link);
  wl_list_remove(&component->link);
  glDeleteVertexArrays(1, &component->vertex_array_id);
  free(component);
}

static void
set_uniform_variables(GLuint program_id, mat4 model, mat4 view, mat4 projection)
{
  mat4 mvp, vp;
  glm_mat4_mul(projection, view, vp);
  glm_mat4_mul(vp, model, mvp);

  GLfloat light_pos[] = {0.0f, 3.0f, 0.0f, 1.0f};
  GLfloat light_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
  GLfloat light_ambient[] = {0.25f, 0.25f, 0.25f, 1.0f};
  GLfloat light_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};

  GLint mvp_matrix_location = glGetUniformLocation(program_id, "zMVP");
  glUniformMatrix4fv(mvp_matrix_location, 1, GL_FALSE, (float *)mvp);

  GLint model_matrix_location = glGetUniformLocation(program_id, "zModel");
  glUniformMatrix4fv(model_matrix_location, 1, GL_FALSE, (float *)model);

  GLint view_matrix_location = glGetUniformLocation(program_id, "zView");
  glUniformMatrix4fv(view_matrix_location, 1, GL_FALSE, (float *)view);

  GLint projection_matrix_location =
      glGetUniformLocation(program_id, "zProjection");
  glUniformMatrix4fv(
      projection_matrix_location, 1, GL_FALSE, (float *)projection);

  GLint vp_matrix_location = glGetUniformLocation(program_id, "zVP");
  glUniformMatrix4fv(vp_matrix_location, 1, GL_FALSE, (float *)vp);

  GLint light_position_location =
      glGetUniformLocation(program_id, "zLight.position");
  glUniform4fv(light_position_location, 1, light_pos);

  GLint light_diffuse_location =
      glGetUniformLocation(program_id, "zLight.diffuse");
  glUniform4fv(light_diffuse_location, 1, light_diffuse);

  GLint light_ambient_location =
      glGetUniformLocation(program_id, "zLight.ambient");
  glUniform4fv(light_ambient_location, 1, light_ambient);

  GLint light_specular_location =
      glGetUniformLocation(program_id, "zLight.specular");
  glUniform4fv(light_specular_location, 1, light_specular);
}

WL_EXPORT void
zen_opengl_component_render(struct zen_opengl_component *component,
    struct zen_opengl_renderer_camera *camera)
{
  struct zen_cuboid_window *cuboid_window;
  struct zen_opengl_element_array_buffer *element_array_buffer;
  struct zen_opengl_shader_program *shader;
  struct zen_opengl_texture *texture;

  if (strcmp(component->virtual_object->role, zen_cuboid_window_role) != 0)
    return;
  cuboid_window = component->virtual_object->role_object;
  element_array_buffer = zen_weak_link_get_user_data(
      &component->current.element_array_buffer_link);
  shader = zen_weak_link_get_user_data(&component->current.shader_program_link);
  texture = zen_weak_link_get_user_data(&component->current.texture_link);

  if (shader == NULL || shader->linked == false) return;

  glBindVertexArray(component->vertex_array_id);
  glUseProgram(shader->program_id);

  set_uniform_variables(shader->program_id,
      cuboid_window->virtual_object->model_matrix, camera->view_matrix,
      camera->projection_matrix);

  if (texture) glBindTexture(GL_TEXTURE_2D, texture->id);

  if (element_array_buffer && GLEW_VERSION_4_5) {
    glVertexArrayElementBuffer(
        component->vertex_array_id, element_array_buffer->id);

    glDrawElements(component->current.topology, component->current.count,
        element_array_buffer->type,
        (const void *)(uintptr_t)component->current.min);
    glVertexArrayElementBuffer(component->vertex_array_id, 0);
  } else if (element_array_buffer) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer->id);
    glDrawElements(component->current.topology, component->current.count,
        element_array_buffer->type,
        (const void *)(uintptr_t)component->current.min);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  } else {
    glDrawArrays(component->current.topology, component->current.min,
        component->current.count);
  }

  glUseProgram(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);
}
