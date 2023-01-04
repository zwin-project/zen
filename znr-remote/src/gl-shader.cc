#include "gl-shader.h"

#include <zen-common.h>

#include "dispatcher.h"

struct znr_gl_shader *
znr_gl_shader_create(struct znr_dispatcher *dispatcher_base, const char *source,
    size_t length, uint32_t type)
{
  auto self = new znr_gl_shader();
  znr_dispatcher_impl *dispatcher =
      zn_container_of(dispatcher_base, dispatcher, base);

  if (self == nullptr) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->proxy = zen::remote::server::CreateGlShader(
      dispatcher->channel, std::string(source, length), type);
  if (!self->proxy) {
    zn_error("Failed to create remote gl shader");
    goto err_delete;
  }

  return self;

err_delete:
  delete self;

err:
  return nullptr;
}

void
znr_gl_shader_destroy(struct znr_gl_shader *self)
{
  delete self;
}
