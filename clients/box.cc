#include <wayland-client.h>

#include "application.h"
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

  return app.Run();
}
