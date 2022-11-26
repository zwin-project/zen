#include <GLES3/gl32.h>
#include <sys/mman.h>
#include <wayland-client.h>
#include <zigen-protocol.h>

#include <cstring>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <iostream>
#include <vector>

#include "application.h"
#include "bounded.h"
#include "buffer.h"
#include "default.vert.h"
#include "fd.h"
#include "gl-base-technique.h"
#include "gl-buffer.h"
#include "gl-program.h"
#include "gl-shader.h"
#include "gl-texture.h"
#include "gl-vertex-array.h"
#include "rendering-unit.h"
#include "shm-pool.h"
#include "texture.fragment.h"
#include "virtual-object.h"

using namespace zen::client;

typedef struct {
  float x, y, z;
  float u, v;
} Vertex;

typedef struct {
  uint8_t r, g, b, a;
} Pixel;

// clang-format off
#define A {-1, +1, -1, 0, 1}
#define B {-1, -1, -1, 0, 0}
#define C {+1, -1, -1, 1, 0}
#define D {+1, +1, -1, 1, 1}
#define E {-1, +1, +1, 0, 1}
#define F {-1, -1, +1, 0, 0}
#define G {+1, -1, +1, 1, 0}
#define H {+1, +1, +1, 1, 1}
// clang-format on

constexpr int kTextureWidth = 64;
constexpr int kTextureHeight = 64;

/** Draw the rotating cube so that it always fits within the boundary */
class Box final : public Bounded
{
 public:
  Box() = delete;
  Box(Application* app) : Bounded(app), app_(app) {}

  ~Box()
  {
    if (pool_fd_ > 0) close(pool_fd_);
    if (pool_mem_ != nullptr) munmap(pool_mem_, pool_size_);
  }

  void SetUniformVariables()
  {
    float rad_2 = M_PI * animation_seed_ * 4;  // rad / 2
    float cos = cosf(rad_2);
    float sin = sinf(rad_2);
    glm::quat quaternion = {0, sin, 0, cos};
    float min = glm::min(glm::min(half_size_.x, half_size_.y), half_size_.z);
    glm::vec3 size(min / sqrt(3));

    glm::mat4 matrix = glm::toMat4(quaternion);

    matrix = glm::scale(matrix, size);

    technique_->Uniform(0, "local_model", matrix);
  }

  void Frame(uint32_t time) override
  {
    animation_seed_ = (float)(time % 12000) / 12000.0f;

    for (int y = 0; y < kTextureHeight; y++) {
      for (int x = 0; x < kTextureWidth; x++) {
        int index = y * kTextureWidth + x;
        texture_mem_[index].r = UINT8_MAX * x / kTextureWidth;
        texture_mem_[index].g = UINT8_MAX * y / kTextureHeight;
        texture_mem_[index].b = (time % 5000) * UINT8_MAX / 5000;
        texture_mem_[index].a = UINT8_MAX;
      }
    }

    texture_->Image2D(GL_TEXTURE_2D, 0, GL_RGBA, kTextureWidth, kTextureHeight,
        0, GL_RGBA, GL_UNSIGNED_BYTE, texture_buffer_.get());

    SetUniformVariables();
    NextFrame();
    Commit();
  }

  void Configure(glm::vec3 half_size, uint32_t serial) override
  {
    half_size_ = half_size;

    SetUniformVariables();

    AckConfigure(serial);

    if (!committed_) {
      NextFrame();
      Commit();
      committed_ = true;
    }
  }

  bool Init()
  {
    if (!Bounded::Init(glm::vec3(0.25, 0.25, 0.50))) return false;

    unit_ = CreateRenderingUnit(app_, this);
    if (!unit_) return false;

    technique_ = CreateGlBaseTechnique(app_, unit_.get());
    if (!technique_) return false;

    ushort a = 0, b = 1, c = 2, d = 3, e = 4, f = 5, g = 6, h = 7;
    vertices_ = {A, B, C, D, E, F, G, H};
    // clang-format off
    elements_ = {
      a, b, c, a, d, c, // front
      e, f, g, e, h, g, // back
    };
    // clang-format on

    vertex_buffer_size_ =
        sizeof(decltype(vertices_)::value_type) * vertices_.size();
    element_array_buffer_size_ =
        sizeof(decltype(elements_)::value_type) * elements_.size();
    size_t texture_buffer_size = sizeof(Pixel) * kTextureWidth * kTextureHeight;
    pool_size_ =
        vertex_buffer_size_ + element_array_buffer_size_ + texture_buffer_size;

    pool_fd_ = create_anonymous_file(pool_size_);
    if (pool_fd_ < 0) return false;

    pool_mem_ = mmap(nullptr, pool_size_, PROT_WRITE, MAP_SHARED, pool_fd_, 0);
    vertex_buffer_mem_ = (Vertex*)pool_mem_;
    element_array_buffer_mem_ =
        (ushort*)((char*)pool_mem_ + vertex_buffer_size_);
    texture_mem_ = (Pixel*)((char*)pool_mem_ + vertex_buffer_size_ +
                            element_array_buffer_size_);

    std::memcpy(vertex_buffer_mem_, vertices_.data(), vertex_buffer_size_);
    std::memcpy(element_array_buffer_mem_, elements_.data(),
        element_array_buffer_size_);

    pool_ = CreateShmPool(app_, pool_fd_, pool_size_);
    if (!pool_) return false;

    vertex_buffer_ = CreateBuffer(pool_.get(), 0, vertex_buffer_size_);
    if (!vertex_buffer_) return false;

    element_array_buffer_ = CreateBuffer(
        pool_.get(), vertex_buffer_size_, element_array_buffer_size_);
    if (!element_array_buffer_) return false;

    texture_buffer_ = CreateBuffer(pool_.get(),
        vertex_buffer_size_ + element_array_buffer_size_, texture_buffer_size);
    if (!texture_buffer_) return false;

    gl_vertex_buffer_ = CreateGlBuffer(app_);
    if (!gl_vertex_buffer_) return false;

    gl_element_array_buffer_ = CreateGlBuffer(app_);
    if (!gl_element_array_buffer_) return false;

    texture_ = CreateGlTexture(app_);
    if (!texture_) return false;

    vertex_array_ = CreateGlVertexArray(app_);
    if (!vertex_array_) return false;

    vertex_shader_ =
        CreateGlShader(app_, GL_VERTEX_SHADER, default_vertex_shader_source);
    if (!vertex_shader_) return false;

    fragment_shader_ = CreateGlShader(
        app_, GL_FRAGMENT_SHADER, texture_fragment_shader_source);
    if (!fragment_shader_) return false;

    program_ = CreateGlProgram(app_);
    if (!program_) return false;

    program_->AttachShader(vertex_shader_.get());
    program_->AttachShader(fragment_shader_.get());
    program_->Link();

    gl_vertex_buffer_->Data(
        GL_ARRAY_BUFFER, vertex_buffer_.get(), GL_STATIC_DRAW);

    gl_element_array_buffer_->Data(
        GL_ELEMENT_ARRAY_BUFFER, element_array_buffer_.get(), GL_STATIC_DRAW);

    for (int y = 0; y < kTextureHeight; y++) {
      for (int x = 0; x < kTextureWidth; x++) {
        int index = y * kTextureWidth + x;
        texture_mem_[index].r = UINT8_MAX * x / kTextureWidth;
        texture_mem_[index].g = UINT8_MAX * y / kTextureHeight;
        texture_mem_[index].b = 0;
        texture_mem_[index].a = UINT8_MAX;
      }
    }

    texture_->Image2D(GL_TEXTURE_2D, 0, GL_RGBA, kTextureWidth, kTextureHeight,
        0, GL_RGBA, GL_UNSIGNED_BYTE, texture_buffer_.get());

    vertex_array_->Enable(0);
    vertex_array_->VertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0, gl_vertex_buffer_.get());
    vertex_array_->Enable(1);
    vertex_array_->VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        offsetof(Vertex, u), gl_vertex_buffer_.get());

    technique_->Bind(vertex_array_.get());
    technique_->Bind(program_.get());
    technique_->Bind(0, "", texture_.get(), GL_TEXTURE_2D);

    technique_->DrawElements(GL_TRIANGLES, elements_.size(), GL_UNSIGNED_SHORT,
        0, gl_element_array_buffer_.get());

    return true;
  }

 private:
  Application* app_;
  std::unique_ptr<RenderingUnit> unit_;
  std::unique_ptr<GlBaseTechnique> technique_;
  std::vector<Vertex> vertices_;
  std::vector<ushort> elements_;
  int pool_fd_ = 0;
  std::unique_ptr<ShmPool> pool_;
  size_t vertex_buffer_size_;
  size_t element_array_buffer_size_;
  size_t pool_size_;
  void* pool_mem_ = nullptr;
  Vertex* vertex_buffer_mem_;
  ushort* element_array_buffer_mem_;
  Pixel* texture_mem_;
  std::unique_ptr<Buffer> vertex_buffer_;
  std::unique_ptr<Buffer> element_array_buffer_;
  std::unique_ptr<Buffer> texture_buffer_;
  std::unique_ptr<GlBuffer> gl_vertex_buffer_;
  std::unique_ptr<GlBuffer> gl_element_array_buffer_;
  std::unique_ptr<GlVertexArray> vertex_array_;
  std::unique_ptr<GlShader> vertex_shader_;
  std::unique_ptr<GlShader> fragment_shader_;
  std::unique_ptr<GlProgram> program_;
  std::unique_ptr<GlTexture> texture_;

  float animation_seed_;  // 0 to 1
  glm::vec3 half_size_;
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
