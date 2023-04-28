#pragma once

#include "gl-base-technique-private.h"
#include "zen-common/cpp-util.h"

namespace zen::backend::immersive::remote {

class GlRenderingUnit;

class GlBaseTechnique
{
 public:
  DISABLE_MOVE_AND_COPY(GlBaseTechnique);
  ~GlBaseTechnique();

  static std::unique_ptr<GlBaseTechnique> New(
      std::shared_ptr<zen::remote::server::IChannel> channel,
      std::unique_ptr<GlRenderingUnit> &rendering_unit);

  inline std::unique_ptr<zen::remote::server::IGlBaseTechnique> &remote_obj();
  inline zn_gl_base_technique *c_obj();

 private:
  GlBaseTechnique() = default;

  bool Init(std::shared_ptr<zen::remote::server::IChannel> channel,
      std::unique_ptr<GlRenderingUnit> &rendering_unit);

  static void HandleBindProgram(struct zn_gl_base_technique *c_obj,
      struct zn_gl_program *gl_program_c_obj);

  static void HandleBindVertexArray(struct zn_gl_base_technique *c_obj,
      struct zn_gl_vertex_array *gl_vertex_array_c_obj);

  static void HandleUniformVector(struct zn_gl_base_technique *c_obj,
      uint32_t location, const char *name, uint32_t type, uint32_t size,
      uint32_t count, void *value);

  static void HandleUniformMatrix(struct zn_gl_base_technique *c_obj,
      uint32_t location, const char *name, uint32_t col, uint32_t row,
      uint32_t count, bool transpose, void *value);

  static void HandleDrawArrays(struct zn_gl_base_technique *c_obj,
      uint32_t mode, int32_t first, uint32_t count);

  static void HandleDrawElements(struct zn_gl_base_technique *c_obj,
      uint32_t mode, uint32_t count, uint32_t type, uint64_t offset,
      struct zn_gl_buffer *gl_buffer_c_obj);

  std::unique_ptr<zen::remote::server::IGlBaseTechnique> remote_obj_;

  static const zn_gl_base_technique_interface c_implementation_;

  zn_gl_base_technique *c_obj_ = nullptr;  // @nonnull after Init(), @owning
};

inline zn_gl_base_technique *
GlBaseTechnique::c_obj()
{
  return c_obj_;
}

inline std::unique_ptr<zen::remote::server::IGlBaseTechnique> &
GlBaseTechnique::remote_obj()
{
  return remote_obj_;
}

}  // namespace zen::backend::immersive::remote
