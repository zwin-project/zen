#include "output.h"

#include <GLFW/glfw3.h>
#include <libzen/libzen.h>
#include <time.h>
#include <wayland-server.h>

#include "compositor.h"
#include "opengl-renderer/opengl-renderer.h"

struct glfw_output {
  struct zen_output base;
  struct zen_compositor* compositor;
  struct zen_opengl_renderer* renderer;

  GLFWwindow* window;

  uint32_t refresh;  // milli hz
  struct wl_event_source* swap_timer;
};

static void
glfw_output_repaint(struct zen_output* zen_output)
{
  struct glfw_output* output = (struct glfw_output*)zen_output;
  zen_opengl_renderer_render(output->renderer);
  // TODO: check if the rendering is in time.
}

static int
swap_timer_loop(void* data)
{
  struct glfw_output* output = data;
  struct timespec next_repaint;
  int64_t refresh_msec;

  glfwSwapBuffers(output->window);

  glfwPollEvents();
  if (glfwWindowShouldClose(output->window))
    wl_display_terminate(output->compositor->display);

  refresh_msec = millihz_to_nsec(output->refresh) / 1000000;
  if (timespec_get(&next_repaint, TIME_UTC) < 0)
    zen_log("glfw output: [WARNING] failed to get current time\n");
  timespec_add_msec(&next_repaint, &next_repaint, refresh_msec);

  wl_event_source_timer_update(output->swap_timer, refresh_msec);

  zen_compositor_complete_frame(output->compositor, next_repaint);

  return 0;
}

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
  struct wl_event_loop* loop;
  struct wl_event_source* swap_timer;
  struct zen_opengl_renderer* renderer;

  window = glfwCreateWindow(640, 480, "Zen Compositor", NULL, NULL);
  if (window == NULL) {
    zen_log("glfw output: failed to create a window\n");
    goto err;
  }

  glfwMakeContextCurrent(window);

  renderer = zen_opengl_renderer_create(compositor);
  if (renderer == NULL) {
    zen_log("glfw backend: failed to create renderer\n");
    goto err_renderer;
  }

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

  loop = wl_display_get_event_loop(compositor->display);
  swap_timer = wl_event_loop_add_timer(loop, swap_timer_loop, output);

  output->base.repaint = glfw_output_repaint;
  output->compositor = compositor;
  output->renderer = renderer;
  output->window = window;
  output->refresh = refresh;
  output->swap_timer = swap_timer;

  // start swap loop
  swap_timer_loop(output);

  return &output->base;

err_output:
  zen_opengl_renderer_destroy(renderer);

err_renderer:
  glfwDestroyWindow(window);

err:
  return NULL;
}

WL_EXPORT void
zen_output_destroy(struct zen_output* output)
{
  struct glfw_output* o = (struct glfw_output*)output;

  wl_event_source_remove(o->swap_timer);
  zen_opengl_renderer_destroy(o->renderer);
  glfwDestroyWindow(o->window);
  free(output);
}
