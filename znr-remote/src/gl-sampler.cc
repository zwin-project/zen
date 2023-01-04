#include "gl-sampler.h"

#include <GLES3/gl32.h>
#include <zen-common.h>

#include <cstring>

#include "dispatcher.h"

void
znr_gl_sampler_parameter_f(
    struct znr_gl_sampler *self, uint32_t pname, float param)
{
  self->proxy->GlSamplerParameterf(pname, param);
}

void
znr_gl_sampler_parameter_i(
    struct znr_gl_sampler *self, uint32_t pname, int32_t param)
{
  self->proxy->GlSamplerParameteri(pname, param);
}

void
znr_gl_sampler_parameter_fv(
    struct znr_gl_sampler *self, uint32_t pname, float *params)
{
  self->proxy->GlSamplerParameterfv(pname, params);
}

void
znr_gl_sampler_parameter_iv(
    struct znr_gl_sampler *self, uint32_t pname, int32_t *params)
{
  self->proxy->GlSamplerParameteriv(pname, params);
}

void
znr_gl_sampler_parameter_iiv(
    struct znr_gl_sampler *self, uint32_t pname, int32_t *params)
{
  self->proxy->GlSamplerParameterIiv(pname, params);
}

void
znr_gl_sampler_parameter_iuiv(
    struct znr_gl_sampler *self, uint32_t pname, uint32_t *params)
{
  self->proxy->GlSamplerParameterIuiv(pname, params);
}

struct znr_gl_sampler *
znr_gl_sampler_create(struct znr_dispatcher *dispatcher_base)
{
  auto self = new znr_gl_sampler();
  znr_dispatcher_impl *dispatcher =
      zn_container_of(dispatcher_base, dispatcher, base);

  if (self == nullptr) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->proxy = zen::remote::server::CreateGlSampler(dispatcher->channel);
  if (!self->proxy) {
    zn_error("Failed to create remote gl sampler");
    goto err_delete;
  }

  return self;

err_delete:
  delete self;

err:
  return nullptr;
}

void
znr_gl_sampler_destroy(struct znr_gl_sampler *self)
{
  delete self;
}
