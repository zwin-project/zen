#include <wayland-client.h>
#include <zigen-protocol.h>

#include "application.h"
#include "fd.h"
#include "gl-base-technique.h"
#include "gl-buffer.h"
#include "rendering-unit.h"
#include "virtual-object.h"

using namespace zen::client;

typedef struct {
  float x, y, z;
} vec3;

int
main(void)
{
  Application app;

  if (!app.Init()) return EXIT_FAILURE;
  if (!app.Connect()) return EXIT_FAILURE;

  auto vo = CreateVirtualObject(&app);
  if (!vo) return EXIT_FAILURE;

  auto unit = CreateRenderingUnit(&app, vo.get());
  if (!unit) return EXIT_FAILURE;

  auto technique = CreateGlBaseTechnique(&app, unit.get());
  if (!technique) return EXIT_FAILURE;

  vec3 vertices[8];

  int fd = create_anonymous_file(sizeof(vertices));
  zgn_shm_pool *pool = zgn_shm_create_pool(app.shm(), fd, sizeof(vertices));
  zgn_buffer *buffer = zgn_shm_pool_create_buffer(pool, 0, sizeof(vertices));
  (void)buffer;

  auto gl_buffer = CreateGlBuffer(&app);
  if (!gl_buffer) return EXIT_FAILURE;

  return app.Run();
}
