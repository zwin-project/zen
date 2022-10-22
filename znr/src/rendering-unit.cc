#include "rendering-unit.h"

#include <zen-common.h>
#include <zen-remote/server/rendering-unit.h>

#include <memory>

#include "gl-buffer.h"
#include "remote.h"

void
znr_rendering_unit_gl_enable_vertex_attrib_array(
    struct znr_rendering_unit* parent, uint32_t index)
{
  znr_rendering_unit_impl* self = zn_container_of(parent, self, base);
  self->proxy->GlEnableVertexAttribArray(index);
}

void
znr_rendering_unit_gl_disable_vertex_attrib_array(
    struct znr_rendering_unit* parent, uint32_t index)
{
  znr_rendering_unit_impl* self = zn_container_of(parent, self, base);
  self->proxy->GlDisableVertexAttribArray(index);
}

void
znr_rendering_unit_gl_vertex_attrib_pointer(struct znr_rendering_unit* parent,
    uint32_t index, uint64_t gl_buffer_id, int32_t size, uint64_t type,
    bool normalized, int32_t stride, uint64_t offset)
{
  znr_rendering_unit_impl* self = zn_container_of(parent, self, base);
  self->proxy->GlVertexAttribPointer(
      index, gl_buffer_id, size, type, normalized, stride, offset);
}

struct znr_rendering_unit*
znr_rendering_unit_create(struct znr_remote* remote, uint64_t virtual_object_id)
{
  znr_rendering_unit_impl* self;
  znr_remote_impl* remote_impl = zn_container_of(remote, remote_impl, base);

  self = new znr_rendering_unit_impl();
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->proxy = zen::remote::server::CreateRenderingUnit(
      remote_impl->proxy, virtual_object_id);
  if (!self->proxy) {
    zn_error("Failed to create a rendering unit");
    goto err_free;
  }

  return &self->base;

err_free:
  delete self;

err:
  return nullptr;
}

void
znr_rendering_unit_destroy(struct znr_rendering_unit* parent)
{
  znr_rendering_unit_impl* self = zn_container_of(parent, self, base);
  self->proxy.reset();
  delete self;
}
