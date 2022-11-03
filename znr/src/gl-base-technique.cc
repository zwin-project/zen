#include "gl-base-technique.h"

#include <zen-common.h>

#include "remote.h"

void
znr_gl_base_technique_gl_draw_arrays(struct znr_gl_base_technique* parent,
    uint32_t mode, int32_t first, uint32_t count)
{
  znr_gl_base_technique_impl* self = zn_container_of(parent, self, base);
  self->proxy->GlDrawArrays(mode, first, count);
}

struct znr_gl_base_technique*
znr_gl_base_technique_create(
    struct znr_remote* remote, uint64_t rendering_unit_id)
{
  znr_gl_base_technique_impl* self;
  znr_remote_impl* remote_impl = zn_container_of(remote, remote_impl, base);

  self = new znr_gl_base_technique_impl();
  if (self == nullptr) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->proxy = zen::remote::server::CreateGlBaseTechnique(
      remote_impl->proxy, rendering_unit_id);
  if (!self->proxy) {
    zn_error("Failed to create a gl base technique");
    goto err_free;
  }

  return &self->base;

err_free:
  delete self;

err:
  return nullptr;
}

void
znr_gl_base_technique_destroy(struct znr_gl_base_technique* parent)
{
  znr_gl_base_technique_impl* self = zn_container_of(parent, self, base);
  self->proxy.reset();
  delete self;
}
