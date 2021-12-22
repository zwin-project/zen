#ifndef ZEN_GLFW_BACKEND_OUTPUT_H
#define ZEN_GLFW_BACKEND_OUTPUT_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <libzen-compositor/libzen-compositor.h>
#include <zen-renderer/opengl-renderer.h>

struct glfw_output {
  struct zen_output base;
  struct zen_compositor* compositor;
  struct zen_opengl_renderer* renderer;

  GLFWwindow* window;
  uint32_t width;
  uint32_t height;

  uint32_t refresh;  // milli hz
  struct wl_event_source* swap_timer;

  struct zen_opengl_renderer_camera eyes[2];  // [left, right]
};

struct zen_output* zen_output_create(struct zen_compositor* compositor);

void zen_output_destroy(struct zen_output* output);

#endif  //  ZEN_GLFW_BACKEND_OUTPUT_H
