#ifndef ZEN_RENDERER_OPENGL_RENDERER_H
#define ZEN_RENDERER_OPENGL_RENDERER_H

#include <GL/glew.h>
#include <cglm/cglm.h>
#include <libzen-compositor/libzen-compositor.h>

extern char* zen_opengl_renderer_type;

struct zen_opengl_renderer {
  struct zen_renderer base;
  struct zen_compositor* compositor;
  struct zen_opengl* opengl;

  struct zen_opengl_renderer_camera* cameras;
  uint32_t camera_count;
  uint32_t camera_allocate;

  struct wl_list cuboid_window_render_item_list;
  struct wl_list component_list;
};

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

struct zen_opengl_renderer* zen_opengl_renderer_get(
    struct zen_renderer* renderer_base);

void zen_opengl_renderer_set_cameras(struct zen_opengl_renderer* renderer,
    struct zen_opengl_renderer_camera* cameras, uint32_t count);

void zen_opengl_renderer_render(struct zen_opengl_renderer* renderer);

#endif  //  ZEN_RENDERER_OPENGL_RENDERER_H
