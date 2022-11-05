#include <wayland-client.h>

#include "application.h"
#include "gl-base-technique.h"
#include "rendering-unit.h"
#include "virtual-object.h"

using namespace zen::client;

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

  return app.Run();
}
