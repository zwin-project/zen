#include "gl-window.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <libzen-compositor/libzen-compositor.h>

static void
GlfwErrorCallback(int code, const char* description)
{
  zen_log("openvr backend - glfw window: %s (%d)\n", description, code);
}

GlWindow::GlWindow() {}

bool
GlWindow::Init(bool fullscreen, bool hidden_cursor)
{
  // primary_monitor & mode will be freed by glfw
  GLFWmonitor* primary_monitor;
  const GLFWvidmode* mode;
  GLenum glewError;

  glfwSetErrorCallback(GlfwErrorCallback);

  if (glfwInit() == GLFW_FALSE) {
    zen_log("openvr backend - gl window: failed to initialize glfw\n");
    goto err;
  }

  primary_monitor = glfwGetPrimaryMonitor();
  if (primary_monitor == NULL) {
    zen_log("openvr backend - gl window: failed to get primary monitor\n");
    goto err_primary_monitor;
  }

  mode = glfwGetVideoMode(primary_monitor);
  if (mode == NULL) {
    zen_log(
        "openvr backend - gl window: failed to get mode of primary monitor\n");
    goto err_mode;
  }

  if (fullscreen) {
    width_ = mode->width;
    height_ = mode->height;

    glfw_window_ =
        glfwCreateWindow(width_, height_, "zen VR View", primary_monitor, NULL);
  } else {
    width_ = 640;
    height_ = 320;
    glfw_window_ = glfwCreateWindow(width_, height_, "zen VR View", NULL, NULL);
  }

  if (glfw_window_ == NULL) {
    zen_log("openvr backend - gl window: failed to create a window\n");
    goto err_window;
  }

  glfwMakeContextCurrent(glfw_window_);
  glfwSwapInterval(0);

  if (hidden_cursor)
    glfwSetInputMode(glfw_window_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

  glewError = glewInit();
  if (glewError != GLEW_OK) {
    zen_log("openvr backend - gl window: failed to initialize glew: %s\n",
        glewGetErrorString(glewError));
    goto err_glew;
  }

  refresh_ = mode->refreshRate * 1000;
  if (refresh_ == 0) {
    zen_log(
        "openvr backend - gl window: [WARNING] failed to get refresh rate. use "
        "60 fps instead.\n");
    refresh_ = 60000;
  }

  return true;

err_glew:
  glfwDestroyWindow(glfw_window_);

err_window:
  glfwTerminate();

err_mode:
err_primary_monitor:
err:
  return false;
}

GlWindow::~GlWindow()
{
  glfwDestroyWindow(glfw_window_);
  glfwTerminate();
}

void
GlWindow::Swap()
{
  glfwSwapBuffers(glfw_window_);
}

bool
GlWindow::Poll()
{
  int32_t width, height;
  glfwPollEvents();

  if (glfwWindowShouldClose(glfw_window_)) return false;
  glfwGetWindowSize(glfw_window_, &width, &height);
  width_ = width;
  height_ = height;

  return true;
}
