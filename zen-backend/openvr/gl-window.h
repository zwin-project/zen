#ifndef ZEN_OPENVR_BACKEND_GL_WINDOW_H
#define ZEN_OPENVR_BACKEND_GL_WINDOW_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <zen-renderer/opengl-renderer.h>

class GlWindow
{
 public:
  GlWindow();
  bool Init(bool fullscreen);
  ~GlWindow();
  void Swap();
  bool Poll();
  inline uint32_t width();
  inline uint32_t height();
  inline uint32_t framebuffer();

 private:
  GLFWwindow *glfw_window_;
  uint32_t refresh_;  // millihz

  GLuint vertex_array_;
  GLuint vertex_buffer_;

  uint32_t width_;
  uint32_t height_;
};

inline uint32_t
GlWindow::width()
{
  return width_;
}

inline uint32_t
GlWindow::height()
{
  return height_;
}

inline uint32_t
GlWindow::framebuffer()
{
  return 0;
}

#endif  //  ZEN_OPENVR_BACKEND_GL_WINDOW_H
