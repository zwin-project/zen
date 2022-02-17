#define _GNU_SOURCE

#include "opengl-shader-program.h"

#include <GL/glew.h>
#include <libzen-compositor/libzen-compositor.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <zigen-opengl-server-protocol.h>

static void zen_opengl_shader_program_destroy(
    struct zen_opengl_shader_program* shader);

enum uniform_variable_type {
  UNIFORM_FLOAT = 0,
};

struct uniform_variable {
  char* location;
  uint32_t col;
  uint32_t row;
  uint32_t type;
  uint32_t count;
  uint32_t transpose;
  void* data;
  uint32_t size;
};

static void
zen_opengl_shader_program_handle_destroy(struct wl_resource* resource)
{
  struct zen_opengl_shader_program* shader;

  shader = wl_resource_get_user_data(resource);

  zen_opengl_shader_program_destroy(shader);
}

static void
zen_opengl_shader_program_protocol_destroy(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);

  wl_resource_destroy(resource);
}

static void
zen_opengl_shader_program_protocol_set_uniform_float_vector(
    struct wl_client* client, struct wl_resource* resource,
    const char* location, uint32_t size, uint32_t count, struct wl_array* value)
{
  UNUSED(client);
  struct zen_opengl_shader_program* shader_program =
      wl_resource_get_user_data(resource);
  struct uniform_variable* variable;

  if (size == 0 || size > 4 || sizeof(float) * size != value->size) {
    wl_resource_post_error(resource,
        ZGN_OPENGL_SHADER_PROGRAM_ERROR_INVALID_UNIFORM_VARIABLE,
        "given uniform vector's size is invalid");
    return;
  }

  variable = wl_array_add(
      &shader_program->pending.uniform_variables, sizeof(*variable));
  variable->location = strdup(location);
  variable->col = 1;
  variable->row = size;
  variable->type = UNIFORM_FLOAT;
  variable->count = count;
  variable->transpose = 0;
  variable->data = malloc(value->size);
  memcpy(variable->data, value->data, value->size);
  variable->size = value->size;
}

static void
zen_opengl_shader_program_protocol_set_uniform_float_matrix(
    struct wl_client* client, struct wl_resource* resource,
    const char* location, uint32_t col, uint32_t row, uint32_t transpose,
    uint32_t count, struct wl_array* value)
{
  UNUSED(client);
  struct zen_opengl_shader_program* shader_program =
      wl_resource_get_user_data(resource);
  struct uniform_variable* variable;

  if (col < 2 || col > 4 || row < 2 || row > 4 ||
      sizeof(float) * col * row != value->size) {
    wl_resource_post_error(resource,
        ZGN_OPENGL_SHADER_PROGRAM_ERROR_INVALID_UNIFORM_VARIABLE,
        "given uniform matrix's size is invalid");
    return;
  }

  variable = wl_array_add(
      &shader_program->pending.uniform_variables, sizeof(*variable));
  variable->location = strdup(location);
  variable->col = col;
  variable->row = row;
  variable->type = UNIFORM_FLOAT;
  variable->count = count;
  variable->transpose = transpose;
  variable->data = malloc(value->size);
  memcpy(variable->data, value->data, value->size);
  variable->size = value->size;
}

static void
zen_opengl_shader_program_protocol_set_vertex_shader(struct wl_client* client,
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
    goto out;
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
    int error_message_length = 0;
    glGetShaderiv(vertex_shader_id, GL_INFO_LOG_LENGTH, &error_message_length);

    char error_message[error_message_length];
    glGetShaderInfoLog(
        vertex_shader_id, error_message_length, NULL, error_message);
    glDeleteShader(vertex_shader_id);

    wl_resource_post_error(resource,
        ZGN_OPENGL_SHADER_PROGRAM_ERROR_COMPILATION_ERROR,
        "failed to compile vertex shader\n%s", error_message);
    goto out;
  }

  if (shader->vertex_shader_id != 0) glDeleteShader(shader->vertex_shader_id);
  shader->vertex_shader_id = vertex_shader_id;

out:
  close(vertex_shader_source_fd);
}

static void
zen_opengl_shader_program_protocol_set_fragment_shader(struct wl_client* client,
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
    goto out;
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
    int error_message_length = 0;
    glGetShaderiv(
        fragment_shader_id, GL_INFO_LOG_LENGTH, &error_message_length);

    char error_message[error_message_length];
    glGetShaderInfoLog(
        fragment_shader_id, error_message_length, NULL, error_message);

    glDeleteShader(fragment_shader_id);
    wl_resource_post_error(resource,
        ZGN_OPENGL_SHADER_PROGRAM_ERROR_COMPILATION_ERROR,
        "failed to compile fragment shader\n%s[%d]", error_message,
        error_message_length);
    goto out;
  }

  if (shader->fragment_shader_id != 0)
    glDeleteShader(shader->fragment_shader_id);
  shader->fragment_shader_id = fragment_shader_id;

out:
  close(fragment_shader_source_fd);
}

static void
zen_opengl_shader_program_protocol_link(
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
    int error_message_length = 0;
    glGetProgramiv(
        shader->program_id, GL_INFO_LOG_LENGTH, &error_message_length);

    char error_message[error_message_length];
    glGetProgramInfoLog(
        shader->program_id, error_message_length, NULL, error_message);
    wl_resource_post_error(resource,
        ZGN_OPENGL_SHADER_PROGRAM_ERROR_LINKAGE_ERROR,
        "failed to link shader programs\n%s", error_message);
    return;
  }
  glUseProgram(shader->program_id);
  glUseProgram(0);
  shader->linked = true;
}

static const struct zgn_opengl_shader_program_interface
    shader_program_interface = {
        .destroy = zen_opengl_shader_program_protocol_destroy,
        .set_uniform_float_vector =
            zen_opengl_shader_program_protocol_set_uniform_float_vector,
        .set_uniform_float_matrix =
            zen_opengl_shader_program_protocol_set_uniform_float_matrix,
        .set_vertex_shader =
            zen_opengl_shader_program_protocol_set_vertex_shader,
        .set_fragment_shader =
            zen_opengl_shader_program_protocol_set_fragment_shader,
        .link = zen_opengl_shader_program_protocol_link,
};

WL_EXPORT void
zen_opengl_shader_program_commit(
    struct zen_opengl_shader_program* shader_program)
{
  struct uniform_variable* variable;

  void (*uniform_matrix_funcs[3][3])(GLint location, GLsizei count,
      GLboolean transpose, const GLfloat* value) = {
      {glUniformMatrix2fv, glUniformMatrix2x3fv, glUniformMatrix2x4fv},
      {glUniformMatrix3x2fv, glUniformMatrix3fv, glUniformMatrix3x4fv},
      {glUniformMatrix4x2fv, glUniformMatrix4x3fv, glUniformMatrix4fv},
  };

  void (*uniform_vector_func[4])(
      GLint location, GLsizei count, const GLfloat* value) = {
      glUniform1fv,
      glUniform2fv,
      glUniform3fv,
      glUniform4fv,
  };

  glUseProgram(shader_program->program_id);
  wl_array_for_each(variable, &shader_program->pending.uniform_variables)
  {
    GLint location =
        glGetUniformLocation(shader_program->program_id, variable->location);

    if (variable->col == 1) {
      uniform_vector_func[variable->row - 1](
          location, variable->count, variable->data);
    } else {
      uniform_matrix_funcs[variable->col - 2][variable->row - 2](
          location, variable->count, variable->transpose, variable->data);
    }

    free(variable->location);
    free(variable->data);
  }
  glUseProgram(0);

  wl_array_release(&shader_program->pending.uniform_variables);
  wl_array_init(&shader_program->pending.uniform_variables);
}

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
      zen_opengl_shader_program_handle_destroy);

  shader->resource = resource;
  shader->program_id = glCreateProgram();
  shader->vertex_shader_id = 0;
  shader->fragment_shader_id = 0;
  shader->linked = false;
  wl_array_init(&shader->pending.uniform_variables);

  return shader;

err_resource:
  free(shader);

err:
  return NULL;
}

static void
zen_opengl_shader_program_destroy(struct zen_opengl_shader_program* shader)
{
  struct uniform_variable* variable;
  wl_array_for_each(variable, &shader->pending.uniform_variables)
  {
    free(variable->location);
    free(variable->data);
  }

  glDeleteProgram(shader->program_id);
  glDeleteShader(shader->vertex_shader_id);
  glDeleteShader(shader->fragment_shader_id);
  wl_array_release(&shader->pending.uniform_variables);
  free(shader);
}
