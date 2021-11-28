#ifndef ZEN_RENDERER_OPENGL_SHADER_COMPILER_H
#define ZEN_RENDERER_OPENGL_SHADER_COMPILER_H

#include <GL/glew.h>

struct zen_opengl_shader_compiler_input {
  GLuint program_id;
  const char *vertex_shader;
  const char *fragment_shader;
};

int zen_opengl_shader_compiler_compile(
    struct zen_opengl_shader_compiler_input *input);

extern const char *zen_opengl_default_vertex_shader;
extern const char *zen_opengl_default_color_fragment_shader;
extern const char *zen_opengl_default_texture_fragment_shader;

#endif  //  ZEN_RENDERER_OPENGL_SHADER_COMPILER_H
