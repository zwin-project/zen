#include "xr-dispatcher.h"

#include <utility>

#include "gl-base-technique.h"
#include "gl-buffer.h"
#include "gl-rendering-unit.h"
#include "gl-shader.h"
#include "virtual-object.h"
#include "zen-common/log.h"
#include "zen/buffer.h"
#include "zen/lease-buffer.h"

namespace zen::backend::immersive::remote {

const zn_xr_dispatcher_interface XrDispatcher::c_implementation_ = {
    XrDispatcher::HandleGetNewVirtualObject,
    XrDispatcher::HandleDestroyVirtualObject,
    XrDispatcher::HandleGetNewGlRenderingUnit,
    XrDispatcher::HandleDestroyGlRenderingUnit,
    XrDispatcher::HandleGetNewGlBaseTechnique,
    XrDispatcher::HandleDestroyGlBaseTechnique,
    XrDispatcher::HandleGetNewGlBuffer,
    XrDispatcher::HandleDestroyGlBuffer,
    XrDispatcher::HandleGetNewGlShader,
    XrDispatcher::HandleDestroyGlShader,
};

XrDispatcher::XrDispatcher(wl_display *display) : display_(display) {}

XrDispatcher::~XrDispatcher()
{
  if (c_obj_ != nullptr) {
    zn_xr_dispatcher_destroy(c_obj_);
  }
}

std::unique_ptr<XrDispatcher>
XrDispatcher::New(
    std::shared_ptr<zen::remote::server::ISession> session, wl_display *display)
{
  auto self = std::unique_ptr<XrDispatcher>(new XrDispatcher(display));
  if (!self) {
    zn_error("Failed to allocate memory");
    return nullptr;
  }

  if (!self->Init(std::move(session))) {
    zn_error("Failed to initialize remote XrDispatcher");
    return nullptr;
  }

  return self;
}

bool
XrDispatcher::Init(std::shared_ptr<zen::remote::server::ISession> session)
{
  c_obj_ = zn_xr_dispatcher_create(this, &c_implementation_);
  if (c_obj_ == nullptr) {
    zn_error("Failed to create zn_xr_dispatcher");
    return false;
  }

  channel_ = zen::remote::server::CreateChannel(session);
  if (!channel_) {
    zn_error("Failed to create a remote channel");
    zn_xr_dispatcher_destroy(c_obj_);
    c_obj_ = nullptr;
    return false;
  }

  return true;
}

zn_virtual_object *
XrDispatcher::HandleGetNewVirtualObject(zn_xr_dispatcher *c_obj)
{
  auto *self = static_cast<XrDispatcher *>(c_obj->impl_data);

  auto virtual_object = VirtualObject::New(self->channel_);
  if (!virtual_object) {
    zn_error("Failed to create a remote virtual object");
    return nullptr;
  }

  auto *virtual_object_c_obj = virtual_object->c_obj();

  self->virtual_objects_.push_back(std::move(virtual_object));

  return virtual_object_c_obj;
}

void
XrDispatcher::HandleDestroyVirtualObject(
    zn_xr_dispatcher *c_obj, zn_virtual_object *virtual_object_c_obj)
{
  auto *self = static_cast<XrDispatcher *>(c_obj->impl_data);

  auto result = std::remove_if(self->virtual_objects_.begin(),
      self->virtual_objects_.end(),
      [virtual_object_c_obj](std::unique_ptr<VirtualObject> &virtual_object) {
        return virtual_object->c_obj() == virtual_object_c_obj;
      });

  self->virtual_objects_.erase(result, self->virtual_objects_.end());
}

zn_gl_rendering_unit *
XrDispatcher::HandleGetNewGlRenderingUnit(
    zn_xr_dispatcher *c_obj, zn_virtual_object *virtual_object_c_obj)
{
  auto *self = static_cast<XrDispatcher *>(c_obj->impl_data);

  for (auto &virtual_object : self->virtual_objects_) {
    if (virtual_object->c_obj() != virtual_object_c_obj) {
      continue;
    }

    auto gl_rendering_unit =
        GlRenderingUnit::New(self->channel_, virtual_object);

    auto *gl_rendering_unit_c_obj = gl_rendering_unit->c_obj();

    self->gl_rendering_units_.push_back(std::move(gl_rendering_unit));

    return gl_rendering_unit_c_obj;
  }

  return nullptr;
}

void
XrDispatcher::HandleDestroyGlRenderingUnit(
    zn_xr_dispatcher *c_obj, zn_gl_rendering_unit *gl_rendering_unit_c_obj)
{
  auto *self = static_cast<XrDispatcher *>(c_obj->impl_data);

  auto result = std::remove_if(self->gl_rendering_units_.begin(),
      self->gl_rendering_units_.end(),
      [gl_rendering_unit_c_obj](
          std::unique_ptr<GlRenderingUnit> &gl_rendering_unit) {
        return gl_rendering_unit->c_obj() == gl_rendering_unit_c_obj;
      });

  self->gl_rendering_units_.erase(result, self->gl_rendering_units_.end());
}

zn_gl_base_technique *
XrDispatcher::HandleGetNewGlBaseTechnique(
    zn_xr_dispatcher *c_obj, zn_gl_rendering_unit *gl_rendering_unit_c_obj)
{
  auto *self = static_cast<XrDispatcher *>(c_obj->impl_data);

  for (auto &rendering_unit : self->gl_rendering_units_) {
    if (rendering_unit->c_obj() != gl_rendering_unit_c_obj) {
      continue;
    }

    auto gl_base_technique =
        GlBaseTechnique::New(self->channel_, rendering_unit);

    auto *gl_base_technique_c_obj = gl_base_technique->c_obj();

    self->gl_base_techniques_.push_back(std::move(gl_base_technique));

    return gl_base_technique_c_obj;
  }

  return nullptr;
}

void
XrDispatcher::HandleDestroyGlBaseTechnique(
    zn_xr_dispatcher *c_obj, zn_gl_base_technique *gl_base_technique_c_obj)
{
  auto *self = static_cast<XrDispatcher *>(c_obj->impl_data);

  auto result = std::remove_if(self->gl_base_techniques_.begin(),
      self->gl_base_techniques_.end(),
      [gl_base_technique_c_obj](
          std::unique_ptr<GlBaseTechnique> &gl_base_technique) {
        return gl_base_technique->c_obj() == gl_base_technique_c_obj;
      });

  self->gl_base_techniques_.erase(result, self->gl_base_techniques_.end());
}

zn_gl_buffer *
XrDispatcher::HandleGetNewGlBuffer(zn_xr_dispatcher *c_obj)
{
  auto *self = static_cast<XrDispatcher *>(c_obj->impl_data);

  auto gl_buffer = GlBuffer::New(self->channel_, self->display_);
  if (!gl_buffer) {
    zn_error("Failed to create a remote gl_buffer");
    return nullptr;
  }

  auto *gl_buffer_c_obj = gl_buffer->c_obj();

  self->gl_buffers_.push_back(std::move(gl_buffer));

  return gl_buffer_c_obj;
}

void
XrDispatcher::HandleDestroyGlBuffer(
    zn_xr_dispatcher *c_obj, struct zn_gl_buffer *gl_buffer_c_obj)
{
  auto *self = static_cast<XrDispatcher *>(c_obj->impl_data);

  auto result =
      std::remove_if(self->gl_buffers_.begin(), self->gl_buffers_.end(),
          [gl_buffer_c_obj](std::unique_ptr<GlBuffer> &gl_buffer) {
            return gl_buffer->c_obj() == gl_buffer_c_obj;
          });

  self->gl_buffers_.erase(result, self->gl_buffers_.end());
}

zn_gl_shader *
XrDispatcher::HandleGetNewGlShader(struct zn_xr_dispatcher *c_obj,
    struct zn_lease_buffer *lease_buffer, uint32_t type)
{
  auto *self = static_cast<XrDispatcher *>(c_obj->impl_data);

  std::string source;

  {
    ssize_t length = zn_buffer_get_size(lease_buffer->buffer);
    auto *data = zn_buffer_begin_access(lease_buffer->buffer);
    source = std::string(static_cast<char *>(data), length);
    zn_buffer_end_access(lease_buffer->buffer);
    zn_lease_buffer_release(lease_buffer);
  }

  auto gl_shader = GlShader::New(self->channel_, std::move(source), type);
  if (!gl_shader) {
    zn_error("Failed to create a remote gl_shader");
    return nullptr;
  }

  auto *gl_shader_c_obj = gl_shader->c_obj();

  self->gl_shaders_.push_back(std::move(gl_shader));

  return gl_shader_c_obj;
}

void
XrDispatcher::HandleDestroyGlShader(
    struct zn_xr_dispatcher *c_obj, struct zn_gl_shader *gl_shader_c_obj)
{
  auto *self = static_cast<XrDispatcher *>(c_obj->impl_data);

  auto result =
      std::remove_if(self->gl_shaders_.begin(), self->gl_shaders_.end(),
          [gl_shader_c_obj](std::unique_ptr<GlShader> &gl_shader) {
            return gl_shader->c_obj() == gl_shader_c_obj;
          });

  self->gl_shaders_.erase(result, self->gl_shaders_.end());
}

}  // namespace zen::backend::immersive::remote
