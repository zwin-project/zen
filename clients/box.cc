#include <GLES3/gl32.h>
#include <sys/mman.h>
#include <wayland-client.h>
#include <zigen-protocol.h>

#include <cstring>
#include <iostream>
#include <vector>

#include "application.h"
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
} vec3;

class Box final : public VirtualObject
{
 public:
  Box() = delete;
  Box(Application* app) : VirtualObject(app), app_(app) {}

  void Frame(uint32_t time) override
  {
    std::cerr << "frame! " << time << std::endl;
    NextFrame();
    Commit();
  }

  bool Init()
  {
    if (!VirtualObject::Init()) return false;

    unit_ = CreateRenderingUnit(app_, this);
    if (!unit_) return false;

    technique_ = CreateGlBaseTechnique(app_, unit_.get());
    if (!technique_) return false;

    float cx = 0, cy = 1.2, cz = -1, size = 0.25;
    // clang-format off
    vertices_ = {
      {cx - size, cy - size, cz + size}, {cx + size, cy - size, cz + size},
      {cx + size, cy - size, cz + size}, {cx + size, cy - size, cz - size},
      {cx + size, cy - size, cz - size}, {cx - size, cy - size, cz - size},
      {cx - size, cy - size, cz - size}, {cx - size, cy - size, cz + size},
      {cx - size, cy + size, cz + size}, {cx + size, cy + size, cz + size},
      {cx + size, cy + size, cz + size}, {cx + size, cy + size, cz - size},
      {cx + size, cy + size, cz - size}, {cx - size, cy + size, cz - size},
      {cx - size, cy + size, cz - size}, {cx - size, cy + size, cz + size},
      {cx - size, cy - size, cz + size}, {cx - size, cy + size, cz + size},
      {cx + size, cy - size, cz + size}, {cx + size, cy + size, cz + size},
      {cx + size, cy - size, cz - size}, {cx + size, cy + size, cz - size},
      {cx - size, cy - size, cz - size}, {cx - size, cy + size, cz - size},
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
  std::vector<vec3> vertices_;
  int vertex_buffer_fd_;
  std::unique_ptr<ShmPool> pool_;
  std::unique_ptr<Buffer> buffer_;
  std::unique_ptr<GlBuffer> gl_buffer_;
  std::unique_ptr<GlVertexArray> vertex_array_;
  std::unique_ptr<GlShader> vertex_shader_;
  std::unique_ptr<GlShader> fragment_shader_;
  std::unique_ptr<GlProgram> program_;
};

int
main(void)
{
  Application app;

  if (!app.Init()) return EXIT_FAILURE;
  if (!app.Connect()) return EXIT_FAILURE;

  Box box(&app);
  if (!box.Init()) return EXIT_FAILURE;

  box.NextFrame();
  box.Commit();

  return app.Run();
}
