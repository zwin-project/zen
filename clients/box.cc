#include <GLES3/gl32.h>
#include <sys/mman.h>
#include <wayland-client.h>
#include <zigen-protocol.h>

#include <cstring>

#include "application.h"
#include "buffer.h"
#include "default.vert.h"
#include "fd.h"
#include "gl-base-technique.h"
#include "gl-buffer.h"
#include "gl-shader.h"
#include "gl-vertex-array.h"
#include "rendering-unit.h"
#include "shm-pool.h"
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

  float cx = 0, cy = 0, cz = 0;
  vec3 vertices[8] = {
      {cx - 1, cy - 1, cz + 1},
      {cx + 1, cy - 1, cz + 1},
      {cx + 1, cy - 1, cz - 1},
      {cx - 1, cy - 1, cz - 1},
      {cx - 1, cy + 1, cz + 1},
      {cx + 1, cy + 1, cz + 1},
      {cx + 1, cy + 1, cz - 1},
      {cx - 1, cy + 1, cz - 1},
  };

  int fd = create_anonymous_file(sizeof(vertices));
  if (fd < 0) return EXIT_FAILURE;

  {
    auto v = mmap(nullptr, sizeof(vertices), PROT_WRITE, MAP_SHARED, fd, 0);
    std::memcpy(v, &vertices[0], sizeof(vertices));
    munmap(v, sizeof(vertices));
  }

  auto pool = CreateShmPool(&app, fd, sizeof(vertices));
  if (!pool) return EXIT_FAILURE;

  auto buffer = CreateBuffer(pool.get(), 0, sizeof(vertices));
  if (!buffer) return EXIT_FAILURE;

  auto gl_buffer = CreateGlBuffer(&app);
  if (!gl_buffer) return EXIT_FAILURE;

  auto vertex_array = CreateGlVertexArray(&app);
  if (!vertex_array) return EXIT_FAILURE;

  auto vertex_shader = CreateGlShader(&app, default_vertex_shader_source);
  if (!vertex_shader) return EXIT_FAILURE;

  gl_buffer->Data(GL_ARRAY_BUFFER, buffer.get(), GL_STATIC_DRAW);

  vertex_array->Enable(0);
  vertex_array->VertexAttribPointer(
      0, 3, GL_FLOAT, GL_FALSE, 0, 0, gl_buffer.get());

  technique->Bind(vertex_array.get());

  vo->Commit();

  return app.Run();
}
