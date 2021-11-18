#include "opengl-renderer.h"

#include <GL/glew.h>
#include <libzen/libzen.h>
#include <wayland-server.h>

struct zen_opengl_renderer {
  struct zen_compositor* compositor;
};

WL_EXPORT struct zen_opengl_renderer*
zen_opengl_renderer_create(struct zen_compositor* compositor)
{
  struct zen_opengl_renderer* renderer;

  renderer = zalloc(sizeof *renderer);
  if (renderer == NULL) {
    zen_log("opengl renderer: failed to create opengl renderer\n");
    goto err;
  }

  renderer->compositor = compositor;

  return renderer;

err:
  return NULL;
}

WL_EXPORT void
zen_opengl_renderer_destroy(struct zen_opengl_renderer* renderer)
{
  free(renderer);
}

WL_EXPORT void
zen_opengl_renderer_set_target()
{}

WL_EXPORT void
zen_opengl_renderer_render(struct zen_opengl_renderer* renderer)
{
  UNUSED(renderer);
  static uint8_t r = 0;
  static uint8_t g = 0;
  static uint8_t b = 0;
  static uint8_t r_del = 1;
  static uint8_t g_del = 5;
  static uint8_t b_del = 7;

  r_del = r == UINT8_MAX ? -1 : r == 0 ? 1 : r_del;
  g_del = g == 250 ? -5 : g == 0 ? 5 : g_del;
  b_del = b == 252 ? -7 : b == 0 ? 7 : b_del;

  r += r_del;
  g += g_del;
  b += b_del;

  glClearColor((float)r / UINT8_MAX, ((float)g / UINT8_MAX),
               ((float)b / UINT8_MAX), 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}
