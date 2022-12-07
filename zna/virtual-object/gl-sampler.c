#include "gl-sampler.h"

#include <zen-common.h>

static void zna_gl_sampler_destroy(struct zna_gl_sampler *self);

static void
zna_gl_sampler_handle_session_destroyed(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zna_gl_sampler *self =
      zn_container_of(listener, self, session_destroyed_listener);

  if (self->znr_gl_sampler) {
    znr_gl_sampler_destroy(self->znr_gl_sampler);
    self->zgnr_gl_sampler = NULL;
  }
}

static void
zna_gl_sampler_handle_zgnr_gl_sampler_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zna_gl_sampler *self =
      zn_container_of(listener, self, zgnr_gl_sampler_destroy_listener);

  zna_gl_sampler_destroy(self);
}

void
zna_gl_sampler_apply_commit(struct zna_gl_sampler *self, bool only_damaged)
{
  struct znr_session *session = self->system->current_session;
  struct zgnr_gl_sampler_parameter *parameter;

  if (self->znr_gl_sampler == NULL) {
    self->znr_gl_sampler = znr_gl_sampler_create(session);
  }

  wl_array_for_each (parameter, &self->zgnr_gl_sampler->current.parameters) {
    if (parameter->damaged || !only_damaged) {
      switch (parameter->type) {
        case ZGNR_GL_SAMPLER_PARAMETER_F:
          znr_gl_sampler_parameter_f(
              self->znr_gl_sampler, parameter->pname, parameter->float_param);
          break;
        case ZGNR_GL_SAMPLER_PARAMETER_I:
          znr_gl_sampler_parameter_i(
              self->znr_gl_sampler, parameter->pname, parameter->int_param);
          break;
        case ZGNR_GL_SAMPLER_PARAMETER_FV:
          znr_gl_sampler_parameter_fv(self->znr_gl_sampler, parameter->pname,
              (float *)parameter->vector_param);
          break;
        case ZGNR_GL_SAMPLER_PARAMETER_IV:
          znr_gl_sampler_parameter_iv(self->znr_gl_sampler, parameter->pname,
              (int32_t *)parameter->vector_param);
          break;
        case ZGNR_GL_SAMPLER_PARAMETER_IIV:
          znr_gl_sampler_parameter_iiv(self->znr_gl_sampler, parameter->pname,
              (int32_t *)parameter->vector_param);
          break;
        case ZGNR_GL_SAMPLER_PARAMETER_IUIV:
          znr_gl_sampler_parameter_iuiv(self->znr_gl_sampler, parameter->pname,
              (uint32_t *)parameter->vector_param);
          break;
      }
    }
  }
}

struct zna_gl_sampler *
zna_gl_sampler_create(
    struct zgnr_gl_sampler *zgnr_gl_sampler, struct zna_system *system)
{
  struct zna_gl_sampler *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zgnr_gl_sampler = zgnr_gl_sampler;
  zgnr_gl_sampler->user_data = self;
  self->system = system;
  self->znr_gl_sampler = NULL;

  self->zgnr_gl_sampler_destroy_listener.notify =
      zna_gl_sampler_handle_zgnr_gl_sampler_destroy;
  wl_signal_add(&self->zgnr_gl_sampler->events.destroy,
      &self->zgnr_gl_sampler_destroy_listener);

  self->session_destroyed_listener.notify =
      zna_gl_sampler_handle_session_destroyed;
  wl_signal_add(&self->system->events.current_session_destroyed,
      &self->session_destroyed_listener);

  return self;

err:
  return NULL;
}

static void
zna_gl_sampler_destroy(struct zna_gl_sampler *self)
{
  if (self->znr_gl_sampler) znr_gl_sampler_destroy(self->znr_gl_sampler);
  wl_list_remove(&self->zgnr_gl_sampler_destroy_listener.link);
  wl_list_remove(&self->session_destroyed_listener.link);
  free(self);
}
