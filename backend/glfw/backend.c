#include <GLFW/glfw3.h>
#include <libzen/libzen.h>
#include <sys/time.h>

struct glfw_backend {
  struct zen_backend base;

  struct zen_compositor* compositor;
};

static void
glfw_error_callback(int code, const char* description)
{
  zen_log("glfw: %s (%d)\n", description, code);
}

WL_EXPORT struct zen_backend*
zen_backend_create(struct zen_compositor* compositor)
{
  struct glfw_backend* backend;
  struct zen_output* output;

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

  backend->base.output = output;
  backend->compositor = compositor;

  return &backend->base;

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

  zen_output_destroy(b->base.output);
  glfwTerminate();
  free(b);
}
