#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <libzen-compositor/libzen-compositor.h>
#include <sys/time.h>

#include "output.h"

struct glfw_backend {
  struct zen_backend base;

  struct zen_compositor* compositor;
  struct zen_udev_seat* udev_seat;
};

static void
glfw_error_callback(int code, const char* description)
{
  zen_log("glfw: %s (%d)\n", description, code);
}

static void
zen_backend_get_head_position(struct zen_backend* backend, vec3 position)
{
  vec3 right, left;
  struct glfw_backend* b = wl_container_of(backend, b, base);
  struct glfw_output* output = wl_container_of(b->base.output, output, base);
  glm_vec4_copy3(output->eyes[0].view_matrix[3], left);
  glm_vec4_copy3(output->eyes[1].view_matrix[3], right);
  glm_vec3_center(left, right, position);
  glm_vec3_negate(position);
}

WL_EXPORT struct zen_backend*
zen_backend_create(struct zen_compositor* compositor)
{
  struct glfw_backend* backend;
  struct zen_output* output;
  struct zen_udev_seat* udev_seat;

  glfwSetErrorCallback(glfw_error_callback);

  if (glfwInit() == GLFW_FALSE) {
    zen_log("glfw backend: failed to initialize glfw\n");
    goto err;
  }

  output = zen_output_create(compositor);
  if (output == NULL) {
    zen_log("glfw backend: failed to create output\n");
    goto err_output;
  }

  backend = zalloc(sizeof *backend);
  if (backend == NULL) {
    zen_log("glfw backend: failed to allocate memory");
    goto err_backend;
  }

  udev_seat = zen_udev_seat_create(compositor);
  if (udev_seat == NULL) {
    zen_log("glfw backend: failed to create a udev seat\n");
    goto err_udev_seat;
  }

  backend->base.output = output;
  backend->base.get_head_position = zen_backend_get_head_position;
  backend->compositor = compositor;
  backend->udev_seat = udev_seat;

  return &backend->base;

err_udev_seat:
  free(backend);

err_backend:
  zen_output_destroy(output);

err_output:
  glfwTerminate();

err:
  return NULL;
}

WL_EXPORT void
zen_backend_destroy(struct zen_backend* backend)
{
  struct glfw_backend* b = (struct glfw_backend*)backend;

  zen_udev_seat_destroy(b->udev_seat);
  zen_output_destroy(b->base.output);
  glfwTerminate();
  free(b);
}
