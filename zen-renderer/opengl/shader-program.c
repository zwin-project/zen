#include "shader-program.h"

#include <GL/glew.h>
#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>

/**
 * Each shader source must be null terminated.
 */
int
zen_opengl_shader_program_compile(struct zen_opengl_shader_program* shader)
{
  GLuint id = glCreateProgram();

  if (shader->vertex_shader == NULL || shader->fragment_shader == NULL) {
    zen_log("opengl shader: vertex shader and fragment shader must be set\n");
    return -1;
  }

  // compile vertex shader
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &shader->vertex_shader, NULL);
  glCompileShader(vertex_shader);

  GLint vertex_shader_compiled = GL_FALSE;
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_shader_compiled);
  if (vertex_shader_compiled != GL_TRUE) {
    zen_log("opengl shader: failed to compile vertex shader\n");
    glDeleteProgram(id);
    glDeleteShader(vertex_shader);
    return -1;
  }
  glAttachShader(id, vertex_shader);
  glDeleteShader(vertex_shader);

  // compile fragment shader
  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &shader->fragment_shader, NULL);
  glCompileShader(fragment_shader);

  GLint fragment_shader_compiled = GL_FALSE;
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_shader_compiled);
  if (fragment_shader_compiled != GL_TRUE) {
    zen_log("opengl shader: failed to compile fragment shader\n");
    glDeleteProgram(id);
    glDeleteShader(fragment_shader);
    return -1;
  }
  glAttachShader(id, fragment_shader);
  glDeleteShader(vertex_shader);

  glLinkProgram(id);

  GLint program_success = GL_FALSE;
  glGetProgramiv(id, GL_LINK_STATUS, &program_success);
  if (program_success != GL_TRUE) {
    zen_log("opengl shader: failed to link program\n");
    glDeleteProgram(id);
    return -1;
  }

  glUseProgram(id);
  glUseProgram(0);

  shader->program_id = id;

  return 0;
}

const char* zen_opengl_default_vertex_shader =
    "#version 410\n"
    "uniform mat4 mvp;\n"
    "layout(location = 0) in vec4 position;\n"
    "layout(location = 1) in vec2 v2UVcoordsIn;\n"
    "layout(location = 2) in vec3 v3NormalIn;\n"
    "out vec2 v2UVcoords;\n"
    "void main()\n"
    "{\n"
    "  v2UVcoords = v2UVcoordsIn;\n"
    "  gl_Position = mvp * position;\n"
    "}\n";

const char* zen_opengl_default_color_fragment_shader =
    "#version 410 core\n"
    "out vec4 outputColor;\n"
    "void main()\n"
    "{\n"
    "  outputColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}\n";

const char* zen_opengl_default_texture_fragment_shader =
    "#version 410 core\n"
    "uniform sampler2D userTexture;\n"
    "in vec2 v2UVcoords;\n"
    "out vec4 outputColor;\n"
    "void main()\n"
    "{\n"
    "  outputColor = texture(userTexture, v2UVcoords);\n"
    "}\n";
