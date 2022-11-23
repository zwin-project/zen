#include <GLES3/gl32.h>
#include <sys/mman.h>
#include <wayland-client.h>
#include <zigen-protocol.h>

#include <cstring>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <iostream>
#include <vector>

#include "application.h"
#include "bounded.h"
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

using namespace zen::client;

typedef struct {
  float x, y, z;
} Vertex;

class Box final : public Bounded
{
 public:
  Box() = delete;
  Box(Application* app) : Bounded(app), app_(app) {}

  void SetUniformVariables()
  {
    glm::vec4 translation = {0, 1.2, -1, 0};

    translation.x = sin(animation_seed_ * 2 * M_PI);

    technique_->Uniform(0, "translate", translation);
  }

  void Frame(uint32_t time) override
  {
    animation_seed_ = (float)(time % 2000) / 2000.0f;

    SetUniformVariables();
    NextFrame();
    Commit();
  }

  void Configure(glm::vec3 half_size, uint32_t serial) override
  {
    (void)half_size;

    AckConfigure(serial);

    if (!committed_) {
      SetUniformVariables();
      NextFrame();
      Commit();
      committed_ = true;
    }
  }

  bool Init()
  {
    if (!Bounded::Init(glm::vec3(0.25, 0.25, 0.25))) return false;

    unit_ = CreateRenderingUnit(app_, this);
    if (!unit_) return false;

    technique_ = CreateGlBaseTechnique(app_, unit_.get());
    if (!technique_) return false;

    float size = 0.25;
    // clang-format off
    vertices_ = {
      {-size, -size, +size}, {+size, -size, +size},
      {+size, -size, +size}, {+size, -size, -size},
      {+size, -size, -size}, {-size, -size, -size},
      {-size, -size, -size}, {-size, -size, +size},
      {-size, +size, +size}, {+size, +size, +size},
      {+size, +size, +size}, {+size, +size, -size},
      {+size, +size, -size}, {-size, +size, -size},
      {-size, +size, -size}, {-size, +size, +size},
      {-size, -size, +size}, {-size, +size, +size},
      {+size, -size, +size}, {+size, +size, +size},
      {+size, -size, -size}, {+size, +size, -size},
      {-size, -size, -size}, {-size, +size, -size},
    };
    // clang-format on

    ssize_t vertex_buffer_size =
        sizeof(decltype(vertices_)::value_type) * vertices_.size();
    vertex_buffer_fd_ = create_anonymous_file(vertex_buffer_size);
    if (vertex_buffer_fd_ < 0) return false;

    {
      auto v = mmap(nullptr, vertex_buffer_size, PROT_WRITE, MAP_SHARED,
          vertex_buffer_fd_, 0);
      std::memcpy(v, vertices_.data(), vertex_buffer_size);
      munmap(v, size);
    }

    pool_ = CreateShmPool(app_, vertex_buffer_fd_, vertex_buffer_size);
    if (!pool_) return false;

    buffer_ = CreateBuffer(pool_.get(), 0, vertex_buffer_size);
    if (!buffer_) return false;

    gl_buffer_ = CreateGlBuffer(app_);
    if (!gl_buffer_) return false;

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

    program_->AttachShader(vertex_shader_.get());
    program_->AttachShader(fragment_shader_.get());
    program_->Link();

    gl_buffer_->Data(GL_ARRAY_BUFFER, buffer_.get(), GL_STATIC_DRAW);

    vertex_array_->Enable(0);
    vertex_array_->VertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 0, 0, gl_buffer_.get());

    technique_->Bind(vertex_array_.get());
    technique_->Bind(program_.get());

    technique_->DrawArrays(GL_LINES, 0, vertices_.size());

    return true;
  }

 private:
  Application* app_;
  std::unique_ptr<RenderingUnit> unit_;
  std::unique_ptr<GlBaseTechnique> technique_;
  std::vector<Vertex> vertices_;
  int vertex_buffer_fd_;
  std::unique_ptr<ShmPool> pool_;
  std::unique_ptr<Buffer> buffer_;
  std::unique_ptr<GlBuffer> gl_buffer_;
  std::unique_ptr<GlVertexArray> vertex_array_;
  std::unique_ptr<GlShader> vertex_shader_;
  std::unique_ptr<GlShader> fragment_shader_;
  std::unique_ptr<GlProgram> program_;

  float animation_seed_;  // 0 to 1
  bool committed_ = false;
};

int
main(void)
{
  Application app;

  if (!app.Init()) return EXIT_FAILURE;
  if (!app.Connect()) return EXIT_FAILURE;

  Box box(&app);
  if (!box.Init()) return EXIT_FAILURE;

  return app.Run();
}
