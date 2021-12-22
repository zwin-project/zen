#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <libzen-compositor/libzen-compositor.h>

#include "output.h"

struct openvr_backend {
  struct zen_backend base;

  struct zen_udev_seat* udev_seat;
};

static void
zen_backend_get_head_position(struct zen_backend* backend, vec3 position)
{
  struct openvr_backend* b = wl_container_of(backend, b, base);
  struct openvr_output* output = wl_container_of(backend->output, output, base);
  output->hmd->GetHeadPosition(position);
}

WL_EXPORT struct zen_backend*
zen_backend_create(struct zen_compositor* compositor)
{
  struct openvr_backend* backend;
  struct zen_output* output;
  struct zen_udev_seat* udev_seat;

  backend = (struct openvr_backend*)zalloc(sizeof *backend);
  if (backend == NULL) {
    zen_log("openvr backend: failed to allocate memory\n");
    goto err;
  }

  output = zen_output_create(compositor);
  if (output == NULL) {
    zen_log("openvr backend: failed to create output\n");
    goto err_backend;
  }

  udev_seat = zen_udev_seat_create(compositor);
  if (udev_seat == NULL) {
    zen_log("openvr backend: failed to create a udev seat\n");
    goto err_udev_seat;
  }

  backend->base.output = output;
  backend->base.get_head_position = zen_backend_get_head_position;
  backend->udev_seat = udev_seat;

  return &backend->base;

err_udev_seat:
  zen_output_destroy(output);

err_backend:
  free(backend);

err:
  return NULL;
}

WL_EXPORT void
zen_backend_destroy(struct zen_backend* backend)
{
  struct openvr_backend* b = (struct openvr_backend*)backend;

  zen_udev_seat_destroy(b->udev_seat);
  zen_output_destroy(b->base.output);
  free(b);
}
