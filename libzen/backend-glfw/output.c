#include "output.h"

#include <GLFW/glfw3.h>
#include <libzen/libzen.h>
#include <time.h>
#include <wayland-server.h>

#include "compositor.h"

struct glfw_output {
  struct zen_output base;

  GLFWwindow* window;
};

static uint32_t
get_refresh_rate()
{
  // primary_monitor & mode will be freed by glfw
  GLFWmonitor* primary_monitor;
  const GLFWvidmode* mode;

  primary_monitor = glfwGetPrimaryMonitor();
  if (primary_monitor == NULL) {
    zen_log("glfw output: failed to get primary monitor\n");
    return 0;
  }

  mode = glfwGetVideoMode(primary_monitor);
  if (mode == NULL) {
    zen_log("glfw output: failed to get mode of primary monitor\n");
    return 0;
  }

  return mode->refreshRate * 1000;
}

WL_EXPORT
struct zen_output*
zen_output_create(struct zen_compositor* compositor)
{
  struct glfw_output* output;
  GLFWwindow* window;
  uint32_t refresh;

  window = glfwCreateWindow(640, 480, "Zen Compositor", NULL, NULL);
  if (window == NULL) {
    zen_log("glfw output: failed to create a window\n");
    goto err;
  }

  glfwMakeContextCurrent(window);

  output = zalloc(sizeof *output);
  if (output == NULL) {
    zen_log("glfw output: failed to allocate memory\n");
    goto err_output;
  }

  refresh = get_refresh_rate();
  if (refresh == 0) {
    zen_log(
        "glfw output: [WARNING] failed to get refresh rate. use 60 fps "
        "instead.\n");
    refresh = 60000;
  }

  output->base.compositor = compositor;
  output->base.refresh = refresh;
  output->window = window;

  return &output->base;

err_output:
  glfwDestroyWindow(window);

err:
  return NULL;
}

WL_EXPORT void
zen_output_destroy(struct zen_output* output)
{
  struct glfw_output* o = (struct glfw_output*)output;

  glfwDestroyWindow(o->window);
  free(output);
}
