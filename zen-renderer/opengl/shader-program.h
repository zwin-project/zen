#ifndef ZEN_RENDERER_OPENGL_SHADER_PROGRAM_H
#define ZEN_RENDERER_OPENGL_SHADER_PROGRAM_H

#include <GL/glew.h>

struct zen_opengl_shader_program {
  GLuint program_id;
  const char *vertex_shader;
  const char *fragment_shader;
};

int zen_opengl_shader_program_compile(struct zen_opengl_shader_program *shader);

extern const char *zen_opengl_default_vertex_shader;
extern const char *zen_opengl_default_color_fragment_shader;
extern const char *zen_opengl_default_texture_fragment_shader;

#endif  //  ZEN_RENDERER_OPENGL_SHADER_PROGRAM_H
