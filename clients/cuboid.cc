#include "cuboid.h"

#include <GLES3/gl32.h>

#include <glm/gtc/matrix_transform.hpp>

#include "buffer.h"
#include "color.fragment.h"
#include "default.vert.h"
#include "fd.h"
#include "gl-base-technique.h"
#include "gl-buffer.h"
#include "gl-program.h"
#include "gl-shader.h"
#include "gl-vertex-array.h"
#include "rendering-unit.h"
#include "shm-pool.h"
#include "virtual-object.h"

namespace zen::client {

Cuboid::Cuboid(VirtualObject* virtual_object)
    : app_(virtual_object->app()), virtual_object_(virtual_object)
{}

Cuboid::~Cuboid() = default;

bool
Cuboid::Render(glm::vec3 half_size, glm::mat4 transform, glm::vec4 color)
{
  if (!initialized_ && Init() == false) {
    return false;
  }

  auto local_model = glm::scale(transform, half_size);
  technique_->Uniform(0, "local_model", local_model);
  technique_->Uniform(0, "color", color);

  return true;
}

bool
Cuboid::Init()
{
  unit_ = CreateRenderingUnit(app_, virtual_object_);
  if (!unit_) return false;

  technique_ = CreateGlBaseTechnique(app_, unit_.get());
  if (!technique_) return false;

  int pool_fd = create_anonymous_file(pool_size());
  if (pool_fd < 0) return false;

  {
    auto vertex_buffer_data = static_cast<char*>(
        mmap(nullptr, pool_size(), PROT_WRITE, MAP_SHARED, pool_fd, 0));
    auto element_buffer_data = vertex_buffer_data + vertex_buffer_size();

    std::memcpy(vertex_buffer_data, vertices_.data(), vertex_buffer_size());
    std::memcpy(element_buffer_data, elements_.data(), element_buffer_size());

    munmap(vertex_buffer_data, pool_size());
  }

  pool_ = CreateShmPool(app_, pool_fd, pool_size());
  close(pool_fd);
  if (!pool_) return false;

  vertex_buffer_ = CreateBuffer(pool_.get(), 0, vertex_buffer_size());
  if (!vertex_buffer_) return false;

  element_array_buffer_ =
      CreateBuffer(pool_.get(), vertex_buffer_size(), element_buffer_size());
  if (!element_array_buffer_) return false;

  gl_vertex_buffer_ = CreateGlBuffer(app_);
  if (!gl_vertex_buffer_) return false;

  gl_element_array_buffer_ = CreateGlBuffer(app_);
  if (!gl_element_array_buffer_) return false;

  vertex_array_ = CreateGlVertexArray(app_);
  if (!vertex_array_) return false;

  vertex_shader_ =
      CreateGlShader(app_, GL_VERTEX_SHADER, default_vertex_shader_source);
  if (!vertex_shader_) return false;

  fragment_shader_ =
      CreateGlShader(app_, GL_FRAGMENT_SHADER, color_fragment_shader_source);
  if (!fragment_shader_) return false;

  program_ = CreateGlProgram(app_);
  if (!program_) return false;

  gl_vertex_buffer_->Data(
      GL_ARRAY_BUFFER, vertex_buffer_.get(), GL_STATIC_DRAW);

  gl_element_array_buffer_->Data(
      GL_ELEMENT_ARRAY_BUFFER, element_array_buffer_.get(), GL_STATIC_DRAW);

  program_->AttachShader(vertex_shader_.get());
  program_->AttachShader(fragment_shader_.get());
  program_->Link();

  vertex_array_->Enable(0);
  vertex_array_->VertexAttribPointer(
      0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0, gl_vertex_buffer_.get());
  vertex_array_->Enable(1);
  vertex_array_->VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      offsetof(Vertex, u), gl_vertex_buffer_.get());

  technique_->Bind(vertex_array_.get());
  technique_->Bind(program_.get());

  technique_->DrawElements(GL_LINES, elements_.size(), GL_UNSIGNED_SHORT, 0,
      gl_element_array_buffer_.get());

  initialized_ = true;

  return true;
}

}  // namespace zen::client
