#include "opengl-component.h"

#include <cglm/cglm.h>
#include <string.h>
#include <wayland-server.h>
#include <zen-shell/zen-shell.h>
#include <zigen-opengl-server-protocol.h>

#include "opengl-shader-program.h"
#include "opengl-vertex-buffer.h"
#include "shader-compiler.h"

WL_EXPORT
void zen_opengl_component_destroy(struct zen_opengl_component *component);

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
  UNUSED(resource);
  UNUSED(texture);
}

static void
zen_opengl_component_protocol_add_vertex_attribute(struct wl_client *client,
    struct wl_resource *resource, uint32_t index, uint32_t size, uint32_t type,
    uint32_t normalized, uint32_t stride, uint32_t pointer)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(index);
  UNUSED(size);
  UNUSED(type);
  UNUSED(normalized);
  UNUSED(stride);
  UNUSED(pointer);
}

static void
zen_opengl_component_protocol_clear_vertex_attribute(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  UNUSED(resource);
}

static void
zen_opengl_component_protocol_set_topology(
    struct wl_client *client, struct wl_resource *resource, uint32_t topology)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(topology);
}

static void
zen_opengl_component_protocol_set_min(
    struct wl_client *client, struct wl_resource *resource, uint32_t min)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(min);
}

static void
zen_opengl_component_protocol_set_count(
    struct wl_client *client, struct wl_resource *resource, uint32_t count)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(count);
}

static const struct zgn_opengl_component_interface opengl_component_interface =
    {
        .destroy = zen_opengl_component_protocol_destroy,
        .attach_vertex_buffer =
            zen_opengl_component_protocol_attach_vertex_buffer,
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

  vertex_buffer =
      zen_weak_link_get_user_data(&component->current.vertex_buffer_link);

  glBindVertexArray(component->vertex_array_id);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer ? vertex_buffer->id : 0);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);  // FIXME:
  glBindVertexArray(0);
  glDisableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void
zen_opengl_component_virtual_object_commit_handler(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zen_opengl_component *component;
  struct zen_opengl_vertex_buffer *vertex_buffer;
  struct zen_opengl_shader_program *shader;
  bool update_vao = false;

  component =
      wl_container_of(listener, component, virtual_object_commit_listener);

  vertex_buffer =
      zen_weak_link_get_user_data(&component->pending.vertex_buffer_link);
  if (vertex_buffer) {
    zen_opengl_vertex_buffer_commit(vertex_buffer);
    zen_weak_link_set(
        &component->current.vertex_buffer_link, vertex_buffer->resource);
    update_vao = true;
  }

  shader = zen_weak_link_get_user_data(&component->pending.shader_program_link);
  if (shader) {
    zen_weak_link_set(
        &component->current.shader_program_link, shader->resource);
  }

  zen_weak_link_unset(&component->pending.vertex_buffer_link);
  zen_weak_link_unset(&component->pending.shader_program_link);

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
  zen_weak_link_init(&component->current.shader_program_link);
  zen_weak_link_init(&component->pending.shader_program_link);

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
  zen_weak_link_unset(&component->current.shader_program_link);
  zen_weak_link_unset(&component->pending.shader_program_link);
  wl_list_remove(&component->virtual_object_commit_listener.link);
  wl_list_remove(&component->virtual_object_destroy_listener.link);
  wl_list_remove(&component->link);
  glDeleteVertexArrays(1, &component->vertex_array_id);
  free(component);
}

WL_EXPORT void
zen_opengl_component_render(struct zen_opengl_component *component,
    struct zen_opengl_renderer_camera *camera)
{
  mat4 mvp;
  struct zen_cuboid_window *cuboid_window;
  struct zen_opengl_shader_program *shader;

  if (strcmp(component->virtual_object->role, zen_cuboid_window_role) != 0)
    return;
  cuboid_window = component->virtual_object->role_object;
  shader = zen_weak_link_get_user_data(&component->current.shader_program_link);

  if (shader == NULL || shader->linked == false) return;

  glm_mat4_copy(cuboid_window->model_matrix, mvp);
  glm_mat4_mul(camera->view_matrix, mvp, mvp);
  glm_mat4_mul(camera->projection_matrix, mvp, mvp);

  glBindVertexArray(component->vertex_array_id);
  glUseProgram(shader->program_id);
  GLint mvp_matrix_location = glGetUniformLocation(shader->program_id, "mvp");
  glUniformMatrix4fv(mvp_matrix_location, 1, GL_FALSE, (float *)mvp);
  glDrawArrays(GL_LINES, 0, 24);  // FIXME:
  glUseProgram(0);
  glBindVertexArray(0);
}
