#ifndef LIBZEN_RENDERER_OPENGL_H
#define LIBZEN_RENDERER_OPENGL_H

#include <GL/glew.h>
#include <cglm/cglm.h>
#include <libzen/libzen.h>

struct zen_opengl_renderer;

struct zen_opengl_renderer* zen_opengl_renderer_create(
    struct zen_compositor* compositor);

void zen_opengl_renderer_destroy(struct zen_opengl_renderer* renderer);

struct zen_opengl_renderer_camera {
  GLuint framebuffer_id;
  struct {
    GLint x;
    GLint y;
    GLsizei width;
    GLsizei height;
  } viewport;
  mat4 view_matrix;
  mat4 projection_matrix;
};

void zen_opengl_renderer_set_cameras(struct zen_opengl_renderer* renderer,
    struct zen_opengl_renderer_camera* cameras, uint32_t count);

void zen_opengl_renderer_render(struct zen_opengl_renderer* renderer);

#endif  //  LIBZEN_RENDERER_OPENGL_H
