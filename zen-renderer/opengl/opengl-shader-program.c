#include "opengl-shader-program.h"

#include <GL/glew.h>
#include <libzen-compositor/libzen-compositor.h>
#include <sys/mman.h>
#include <zigen-opengl-server-protocol.h>

static void zen_opengl_shader_program_destroy(
    struct zen_opengl_shader_program* shader);

static void
zgn_opengl_shader_program_handle_destroy(struct wl_resource* resource)
{
  struct zen_opengl_shader_program* shader;

  shader = wl_resource_get_user_data(resource);

  zen_opengl_shader_program_destroy(shader);
}

static void
zgn_opengl_shader_program_protocol_destroy(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);

  wl_resource_destroy(resource);
}

static void
zgn_opengl_shader_program_protocol_set_vertex_shader(struct wl_client* client,
    struct wl_resource* resource, int32_t vertex_shader_source_fd,
    uint32_t vertex_shader_size)
{
  UNUSED(client);
  struct zen_opengl_shader_program* shader;
  char* source;

  shader = wl_resource_get_user_data(resource);

  if (shader->linked) {
    wl_resource_post_error(resource,
        ZGN_OPENGL_SHADER_PROGRAM_ERROR_IMMUTABLE_ERROR,
        "cannot update vertex shader once the programs are linked");
    return;
  }

  source = mmap(NULL, vertex_shader_size, PROT_READ, MAP_SHARED,
      vertex_shader_source_fd, 0);
  GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader_id, 1, (const GLchar* const*)&source,
      (GLint*)&vertex_shader_size);
  munmap(source, vertex_shader_size);
  glCompileShader(vertex_shader_id);

  GLint vertex_shader_compiled = GL_FALSE;
  glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &vertex_shader_compiled);
  if (vertex_shader_compiled != GL_TRUE) {
    glDeleteShader(vertex_shader_id);
    wl_resource_post_error(resource,
        ZGN_OPENGL_SHADER_PROGRAM_ERROR_COMPILATION_ERROR,
        "failed to compile vertex shader");
    return;
  }

  if (shader->vertex_shader_id != 0) glDeleteShader(shader->vertex_shader_id);
  shader->vertex_shader_id = vertex_shader_id;
}

static void
zgn_opengl_shader_program_protocol_set_fragment_shader(struct wl_client* client,
    struct wl_resource* resource, int32_t fragment_shader_source_fd,
    uint32_t fragment_shader_size)
{
  UNUSED(client);
  struct zen_opengl_shader_program* shader;
  char* source;

  shader = wl_resource_get_user_data(resource);

  if (shader->linked) {
    wl_resource_post_error(resource,
        ZGN_OPENGL_SHADER_PROGRAM_ERROR_IMMUTABLE_ERROR,
        "cannot update fragment shader once the programs are linked");
    return;
  }

  source = mmap(NULL, fragment_shader_size, PROT_READ, MAP_SHARED,
      fragment_shader_source_fd, 0);
  GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader_id, 1, (const GLchar* const*)&source,
      (GLint*)&fragment_shader_size);
  munmap(source, fragment_shader_size);
  glCompileShader(fragment_shader_id);

  GLint fragment_shader_compiled = GL_FALSE;
  glGetShaderiv(
      fragment_shader_id, GL_COMPILE_STATUS, &fragment_shader_compiled);
  if (fragment_shader_compiled != GL_TRUE) {
    glDeleteShader(fragment_shader_id);
    wl_resource_post_error(resource,
        ZGN_OPENGL_SHADER_PROGRAM_ERROR_COMPILATION_ERROR,
        "failed to compile fragment shader");
    return;
  }

  if (shader->fragment_shader_id != 0)
    glDeleteShader(shader->fragment_shader_id);
  shader->fragment_shader_id = fragment_shader_id;
}

static void
zgn_opengl_shader_program_protocol_link(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  struct zen_opengl_shader_program* shader;

  shader = wl_resource_get_user_data(resource);

  glAttachShader(shader->program_id, shader->vertex_shader_id);
  glAttachShader(shader->program_id, shader->fragment_shader_id);

  glDeleteShader(shader->vertex_shader_id);
  shader->vertex_shader_id = 0;
  glDeleteShader(shader->fragment_shader_id);
  shader->fragment_shader_id = 0;

  glLinkProgram(shader->program_id);

  GLint link_success = GL_FALSE;
  glGetProgramiv(shader->program_id, GL_LINK_STATUS, &link_success);
  if (link_success != GL_TRUE) {
    wl_resource_post_error(resource,
        ZGN_OPENGL_SHADER_PROGRAM_ERROR_LINKAGE_ERROR,
        "failed to link shader programs");
    return;
  }
  glUseProgram(shader->program_id);
  glUseProgram(0);
  shader->linked = true;
}

static const struct zgn_opengl_shader_program_interface
    shader_program_interface = {
        .destroy = zgn_opengl_shader_program_protocol_destroy,
        .set_vertex_shader =
            zgn_opengl_shader_program_protocol_set_vertex_shader,
        .set_fragment_shader =
            zgn_opengl_shader_program_protocol_set_fragment_shader,
        .link = zgn_opengl_shader_program_protocol_link,
};

WL_EXPORT struct zen_opengl_shader_program*
zen_opengl_shader_program_create(struct wl_client* client, uint32_t id)
{
  struct zen_opengl_shader_program* shader;
  struct wl_resource* resource;

  shader = zalloc(sizeof *shader);
  if (shader == NULL) {
    wl_client_post_no_memory(client);
    zen_log("opengl shader: failed to allocate memory\n");
    goto err;
  }

  resource =
      wl_resource_create(client, &zgn_opengl_shader_program_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    zen_log("opengl shader: failed to create a resource\n");
    goto err_resource;
  }

  wl_resource_set_implementation(resource, &shader_program_interface, shader,
      zgn_opengl_shader_program_handle_destroy);

  shader->resource = resource;
  shader->program_id = glCreateProgram();
  shader->vertex_shader_id = 0;
  shader->fragment_shader_id = 0;
  shader->linked = false;

  return shader;

err_resource:
  free(shader);

err:
  return NULL;
}

static void
zen_opengl_shader_program_destroy(struct zen_opengl_shader_program* shader)
{
  glDeleteProgram(shader->program_id);
  glDeleteShader(shader->vertex_shader_id);
  glDeleteShader(shader->fragment_shader_id);
  free(shader);
}
