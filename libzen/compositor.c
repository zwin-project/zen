#include "compositor.h"

#include <libzen/libzen.h>
#include <stdio.h>
#include <wayland-server.h>

#include "backend.h"

WL_EXPORT int
zen_compositor_load_backend(struct zen_compositor *compositor)
{
  struct zen_backend *backend;

  backend = zen_backend_create(compositor);
  if (backend == NULL) {
    zen_log("compositor: failed to create a backend\n");
    goto err;
  }

  compositor->backend = backend;

  return 0;

err:
  return -1;
}

WL_EXPORT struct zen_compositor *
zen_compositor_create(struct wl_display *display)
{
  struct zen_compositor *compositor;
  struct wl_global *global;
  UNUSED(global);

  compositor = zalloc(sizeof *compositor);
  if (compositor == NULL) {
    zen_log("compositor: failed to allcate memory\n");
    goto out;
  }

  compositor->display = display;
  compositor->backend = NULL;

  return compositor;

out:
  return NULL;
}

WL_EXPORT void
zen_compositor_destroy(struct zen_compositor *compositor)
{
  if (compositor->backend) zen_backend_destroy(compositor->backend);
  free(compositor);
}
