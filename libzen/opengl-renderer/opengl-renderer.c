#include "opengl-renderer.h"

#include <GL/glew.h>
#include <libzen/libzen.h>
#include <string.h>
#include <wayland-server.h>

#include "opengl.h"

struct zen_opengl_renderer {
  struct zen_compositor* compositor;
  struct zen_opengl* opengl;

  struct zen_opengl_renderer_camera* cameras;
  uint32_t camera_count;
  uint32_t camera_allocate;
};

WL_EXPORT struct zen_opengl_renderer*
zen_opengl_renderer_create(struct zen_compositor* compositor)
{
  struct zen_opengl_renderer* renderer;
  struct zen_opengl* opengl;

  renderer = zalloc(sizeof *renderer);
  if (renderer == NULL) {
    zen_log("opengl renderer: failed to create opengl renderer\n");
    goto err;
  }

  opengl = zen_opengl_create(compositor);
  if (opengl == NULL) {
    zen_log("opengl renderer: failed to create zen opengl\n");
    goto err_opengl;
  }

  renderer->compositor = compositor;
  renderer->opengl = opengl;
  renderer->cameras = NULL;
  renderer->camera_count = 0;
  renderer->camera_allocate = 0;

  return renderer;

err_opengl:
  free(renderer);

err:
  return NULL;
}

WL_EXPORT void
zen_opengl_renderer_destroy(struct zen_opengl_renderer* renderer)
{
  if (renderer->camera_allocate > 0) free(renderer->cameras);
  zen_opengl_destroy(renderer->opengl);
  free(renderer);
}

WL_EXPORT void
zen_opengl_renderer_set_cameras(struct zen_opengl_renderer* renderer,
    struct zen_opengl_renderer_camera* cameras, uint32_t count)
{
  size_t size = sizeof(struct zen_opengl_renderer_camera) * count;
  if (count > renderer->camera_allocate) {
    free(renderer->cameras);
    renderer->cameras = malloc(size);
    renderer->camera_allocate = count;
  }
  memcpy(renderer->cameras, cameras, size);
  renderer->camera_count = count;
}

WL_EXPORT void
zen_opengl_renderer_render(struct zen_opengl_renderer* renderer)
{
  struct zen_opengl_renderer_camera* camera = renderer->cameras;

  if (renderer->camera_count == 0) {
    static bool warned = false;
    if (!warned)
      zen_log(
          "opengl renderer: [WARNING] tried to render without any camera\n");
    warned = true;
    return;
  }

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  for (; camera < renderer->cameras + renderer->camera_count; camera++) {
    glBindFramebuffer(GL_FRAMEBUFFER, camera->framebuffer_id);
    glViewport(camera->viewport.x, camera->viewport.y, camera->viewport.width,
        camera->viewport.height);
    // TODO: rendering
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  renderer->camera_count = 0;
}
