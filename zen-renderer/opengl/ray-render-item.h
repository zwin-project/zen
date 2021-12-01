#ifndef ZEN_RENDERER_OPENGL_RAY_RENDER_ITEM_H
#define ZEN_RENDERER_OPENGL_RAY_RENDER_ITEM_H

#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>
#include <zen-renderer/opengl-renderer.h>

struct zen_opengl_ray_render_item {
  struct zen_render_item base;
  struct zen_opengl_renderer *renderer;

  struct zen_ray *ray;

  struct wl_list link;

  GLuint vertex_array_id;
  GLuint vertex_buffer_id;
  GLuint program_id;
  vec3 vertex_buffer[2];
};

void zen_opengl_ray_render_item_render(
    struct zen_opengl_ray_render_item *render_item,
    struct zen_opengl_renderer_camera *camera);

#endif  //  ZEN_RENDERER_OPENGL_RAY_RENDER_ITEM_H
