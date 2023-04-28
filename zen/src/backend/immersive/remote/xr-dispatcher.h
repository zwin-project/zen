#pragma once

#include "xr-dispatcher-private.h"
#include "zen-common/cpp-util.h"

namespace zen::backend::immersive::remote {

class GlBaseTechnique;
class GlBuffer;
class GlProgram;
class GlRenderingUnit;
class GlShader;
class GlTexture;
class GlVertexArray;
class VirtualObject;

class XrDispatcher
{
 public:
  DISABLE_MOVE_AND_COPY(XrDispatcher);
  ~XrDispatcher();

  static std::unique_ptr<XrDispatcher> New(
      std::shared_ptr<zen::remote::server::ISession> session,
      wl_display *display);

  inline zn_xr_dispatcher *c_obj();

 private:
  explicit XrDispatcher(wl_display *display);

  bool Init(std::shared_ptr<zen::remote::server::ISession> session);

  static zn_virtual_object *HandleGetNewVirtualObject(zn_xr_dispatcher *c_obj);

  static void HandleDestroyVirtualObject(
      zn_xr_dispatcher *c_obj, zn_virtual_object *virtual_object_c_obj);

  static zn_gl_rendering_unit *HandleGetNewGlRenderingUnit(
      zn_xr_dispatcher *c_obj, zn_virtual_object *virtual_object_c_obj);

  static void HandleDestroyGlRenderingUnit(
      zn_xr_dispatcher *c_obj, zn_gl_rendering_unit *gl_rendering_unit_c_obj);

  static zn_gl_base_technique *HandleGetNewGlBaseTechnique(
      zn_xr_dispatcher *c_obj, zn_gl_rendering_unit *gl_rendering_unit_c_obj);

  static void HandleDestroyGlBaseTechnique(
      zn_xr_dispatcher *c_obj, zn_gl_base_technique *gl_base_technique_c_obj);

  static zn_gl_buffer *HandleGetNewGlBuffer(zn_xr_dispatcher *c_obj);

  static void HandleDestroyGlBuffer(
      zn_xr_dispatcher *c_obj, struct zn_gl_buffer *gl_buffer_c_obj);

  static zn_gl_shader *HandleGetNewGlShader(
      struct zn_xr_dispatcher *c_obj, struct zn_buffer *buffer, uint32_t type);

  static void HandleDestroyGlShader(
      struct zn_xr_dispatcher *c_obj, struct zn_gl_shader *gl_shader_c_obj);

  static zn_gl_program *HandleGetNewGlProgram(struct zn_xr_dispatcher *c_obj);

  static void HandleDestroyGlProgram(
      struct zn_xr_dispatcher *c_obj, struct zn_gl_program *gl_program_c_obj);

  static zn_gl_texture *HandleGetNewGlTexture(struct zn_xr_dispatcher *c_obj);

  static void HandleDestroyGlTexture(
      struct zn_xr_dispatcher *c_obj, struct zn_gl_texture *gl_texture_c_obj);

  static zn_gl_vertex_array *HandleGetNewGlVertexArray(
      struct zn_xr_dispatcher *c_obj);

  static void HandleDestroyGlVertexArray(struct zn_xr_dispatcher *c_obj,
      struct zn_gl_vertex_array *gl_vertex_array_c_obj);

  wl_display *display_;  // @nonnull, @outlive

  std::shared_ptr<zen::remote::server::IChannel> channel_;

  std::vector<std::unique_ptr<VirtualObject>> virtual_objects_;

  std::vector<std::unique_ptr<GlRenderingUnit>> gl_rendering_units_;

  std::vector<std::unique_ptr<GlBaseTechnique>> gl_base_techniques_;

  std::vector<std::unique_ptr<GlBuffer>> gl_buffers_;

  std::vector<std::unique_ptr<GlShader>> gl_shaders_;

  std::vector<std::unique_ptr<GlProgram>> gl_programs_;

  std::vector<std::unique_ptr<GlTexture>> gl_textures_;

  std::vector<std::unique_ptr<GlVertexArray>> gl_vertex_arrays_;

  static const zn_xr_dispatcher_interface c_implementation_;

  zn_xr_dispatcher *c_obj_ = nullptr;  // @nonnull after Init(), @owning
};

inline zn_xr_dispatcher *
XrDispatcher::c_obj()
{
  return c_obj_;
}

}  // namespace zen::backend::immersive::remote
