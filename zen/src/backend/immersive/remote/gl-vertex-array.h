#pragma once

#include "gl-vertex-array-private.h"
#include "zen-common/cpp-util.h"

namespace zen::backend::immersive::remote {

class GlVertexArray
{
 public:
  DISABLE_MOVE_AND_COPY(GlVertexArray);
  ~GlVertexArray();

  static std::unique_ptr<GlVertexArray> New(
      std::shared_ptr<zen::remote::server::IChannel> channel);

  inline std::unique_ptr<zen::remote::server::IGlVertexArray> &remote_obj();
  inline zn_gl_vertex_array *c_obj() const;

 private:
  GlVertexArray() = default;

  bool Init(std::shared_ptr<zen::remote::server::IChannel> channel);

  static void HandleEnableVertexAttribArray(
      struct zn_gl_vertex_array *c_obj, uint32_t index);

  static void HandleDisableVertexAttribArray(
      struct zn_gl_vertex_array *c_obj, uint32_t index);

  static void HandleVertexAttribPointer(struct zn_gl_vertex_array *c_obj,
      uint32_t index, int32_t size, uint32_t type, bool normalized,
      int32_t stride, uint64_t offset, zn_gl_buffer *gl_buffer_c_obj);

  std::unique_ptr<zen::remote::server::IGlVertexArray> remote_obj_;

  static const zn_gl_vertex_array_interface c_implementation_;

  zn_gl_vertex_array *c_obj_ = nullptr;  // @nonnull after Init(), @owning
};

inline std::unique_ptr<zen::remote::server::IGlVertexArray> &
GlVertexArray::remote_obj()
{
  return remote_obj_;
}

inline zn_gl_vertex_array *
GlVertexArray::c_obj() const
{
  return c_obj_;
}

}  // namespace zen::backend::immersive::remote
