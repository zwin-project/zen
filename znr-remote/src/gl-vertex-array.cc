#include "gl-vertex-array.h"

#include <zen-common.h>

#include "dispatcher.h"
#include "gl-buffer.h"

void
znr_gl_vertex_array_enable_vertex_attrib_array(
    struct znr_gl_vertex_array *self, uint32_t index)
{
  self->proxy->GlEnableVertexAttribArray(index);
}

void
znr_gl_vertex_array_disable_vertex_attrib_array(
    struct znr_gl_vertex_array *self, uint32_t index)
{
  self->proxy->GlDisableVertexAttribArray(index);
}

void
znr_gl_vertex_array_vertex_attrib_pointer(struct znr_gl_vertex_array *self,
    uint32_t index, int32_t size, uint32_t type, bool normalized,
    int32_t stride, uint64_t offset, struct znr_gl_buffer *gl_buffer)
{
  self->proxy->GlVertexAttribPointer(
      index, size, type, normalized, stride, offset, gl_buffer->proxy->id());
}

struct znr_gl_vertex_array *
znr_gl_vertex_array_create(struct znr_dispatcher *dispatcher_base)
{
  auto self = new znr_gl_vertex_array();
  znr_dispatcher_impl *dispatcher =
      zn_container_of(dispatcher_base, dispatcher, base);

  if (self == nullptr) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->proxy = zen::remote::server::CreateGlVertexArray(dispatcher->channel);
  if (!self->proxy) {
    zn_error("Failed to create remote gl vertex array");
    goto err_delete;
  }

  return self;

err_delete:
  delete self;

err:
  return nullptr;
}

void
znr_gl_vertex_array_destroy(struct znr_gl_vertex_array *self)
{
  delete self;
}
