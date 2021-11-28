#ifndef ZEN_RENDERER_OPENGL_SHADER_PROGRAM_H
#define ZEN_RENDERER_OPENGL_SHADER_PROGRAM_H

#include <GL/glew.h>
#include <wayland-server.h>

struct zen_opengl_shader_program {
  struct wl_resource* resource;

  GLuint program_id;
  GLuint vertex_shader_id;
  GLuint fragment_shader_id;
  bool linked;
};

struct zen_opengl_shader_program* zen_opengl_shader_program_create(
    struct wl_client* client, uint32_t id);

#endif  //  ZEN_RENDERER_OPENGL_SHADER_PROGRAM_H
