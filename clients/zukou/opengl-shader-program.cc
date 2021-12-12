#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <zukou.h>

namespace zukou {

static int
create_shared_fd(off_t size)
{
  const char *name = "zen-shader";
  int fd = memfd_create(name, MFD_CLOEXEC | MFD_ALLOW_SEALING);
  if (fd < 0) return fd;
  unlink(name);

  if (ftruncate(fd, size) < 0) {
    close(fd);
    return -1;
  }

  return fd;
}

OpenGLShaderProgram::OpenGLShaderProgram(App *app)
{
  shader_ = zgn_opengl_create_shader_program(app->opengl());
  vertex_shader_fd_ = 0;
  fragment_shader_fd_ = 0;
}

OpenGLShaderProgram::~OpenGLShaderProgram()
{
  if (vertex_shader_fd_ != 0) close(vertex_shader_fd_);
  if (fragment_shader_fd_ != 0) close(fragment_shader_fd_);
  zgn_opengl_shader_program_destroy(shader_);
}

void
OpenGLShaderProgram::SetUniformVariable(const char *location, glm::mat4 mat)
{
  struct wl_array array;
  wl_array_init(&array);
  glm_mat4_to_wl_array(mat, &array);
  zgn_opengl_shader_program_set_uniform_float_matrix(
      shader(), location, 4, 4, false, 1, &array);
  wl_array_release(&array);
}

void
OpenGLShaderProgram::SetUniformVariable(const char *location, glm::vec4 vec)
{
  struct wl_array array;
  wl_array_init(&array);
  glm_vec4_to_wl_array(vec, &array);
  zgn_opengl_shader_program_set_uniform_float_vector(
      shader(), location, 4, 1, &array);
  wl_array_release(&array);
}

void
OpenGLShaderProgram::SetUniformVariable(const char *location, glm::vec3 vec)
{
  struct wl_array array;
  wl_array_init(&array);
  glm_vec3_to_wl_array(vec, &array);
  zgn_opengl_shader_program_set_uniform_float_vector(
      shader(), location, 3, 1, &array);
  wl_array_release(&array);
}

bool
OpenGLShaderProgram::SetVertexShader(const char *source, size_t len)
{
  vertex_shader_fd_ = create_shared_fd(len);
  void *data = mmap(NULL, len, PROT_WRITE, MAP_SHARED, vertex_shader_fd_, 0);
  if (data == MAP_FAILED) {
    close(vertex_shader_fd_);
    vertex_shader_fd_ = 0;
    return false;
  }
  memcpy(data, source, len);
  munmap(data, len);

  zgn_opengl_shader_program_set_vertex_shader(shader_, vertex_shader_fd_, len);

  return true;
}

bool
OpenGLShaderProgram::SetFragmentShader(const char *source, size_t len)
{
  fragment_shader_fd_ = create_shared_fd(len);
  void *data = mmap(NULL, len, PROT_WRITE, MAP_SHARED, fragment_shader_fd_, 0);
  if (data == MAP_FAILED) {
    close(fragment_shader_fd_);
    fragment_shader_fd_ = 0;
    return false;
  }
  memcpy(data, source, len);
  munmap(data, len);

  zgn_opengl_shader_program_set_fragment_shader(
      shader_, fragment_shader_fd_, len);

  return true;
}

void
OpenGLShaderProgram::Link()
{
  zgn_opengl_shader_program_link(shader_);
}
}  // namespace zukou
