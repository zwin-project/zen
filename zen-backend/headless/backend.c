#include <libzen-compositor/libzen-compositor.h>

#include "output.h"

struct headless_backend {
  struct zen_backend base;
};

static void
zen_backend_get_head_position(struct zen_backend* backend, vec3 position)
{
  UNUSED(backend);
  position[0] = 0;
  position[1] = 0;
  position[2] = 1;
}

WL_EXPORT struct zen_backend*
zen_backend_create(struct zen_compositor* compositor)
{
  struct headless_backend* backend;
  struct zen_output* output;

  output = zen_output_create(compositor);
  if (output == NULL) {
    zen_log("headless backend: failed to create output\n");
    goto err;
  }

  backend = zalloc(sizeof *backend);
  if (backend == NULL) {
    zen_log("headless backend: failed to allocate memory\n");
    goto err_backend;
  }

  backend->base.output = output;
  backend->base.get_head_position = zen_backend_get_head_position;

  return &backend->base;

err_backend:
  zen_output_destroy(output);

err:
  return NULL;
}

WL_EXPORT void
zen_backend_destroy(struct zen_backend* backend)
{
  struct headless_backend* b = (struct headless_backend*)backend;

  zen_output_destroy(b->base.output);
  free(b);
}
