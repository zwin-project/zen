#include "gl-sampler.h"

#include <GLES3/gl32.h>
#include <zen-common.h>
#include <zwin-gles-v32-protocol.h>
#include <zwin-protocol.h>

static void zwnr_gl_sampler_destroy(struct zwnr_gl_sampler_impl *self);

/** return 0 if pname is invalid */
static size_t
gl_sampler_parameter_size(uint32_t pname)
{
  switch (pname) {
    case GL_TEXTURE_MIN_FILTER:
    case GL_TEXTURE_MAG_FILTER:
    case GL_TEXTURE_WRAP_S:
    case GL_TEXTURE_WRAP_T:
    case GL_TEXTURE_WRAP_R:
    case GL_TEXTURE_MIN_LOD:
    case GL_TEXTURE_MAX_LOD:
    case GL_TEXTURE_COMPARE_MODE:
    case GL_TEXTURE_COMPARE_FUNC:
      return 4;

    case GL_TEXTURE_BORDER_COLOR:
      return 16;

    default:
      return 0;
  }
}

static void
zwnr_gl_sampler_handle_destroy(struct wl_resource *resource)
{
  struct zwnr_gl_sampler_impl *self = wl_resource_get_user_data(resource);

  zwnr_gl_sampler_destroy(self);
}

static void
zwnr_gl_sampler_set_parameter(struct zwnr_gl_sampler_impl *self,
    enum zwnr_gl_sampler_parameter_type type, uint32_t pname, float float_param,
    int32_t int_param, struct wl_array *vector_param,
    struct wl_resource *error_resource)
{
  size_t expected_size = gl_sampler_parameter_size(pname);
  size_t vector_param_size = vector_param ? vector_param->size : 0;

  bool invalid_pname = false, unexpected_wl_array_size = false;
  switch (type) {
    case ZWNR_GL_SAMPLER_PARAMETER_F:
      if (expected_size != sizeof(float)) invalid_pname = true;
      break;
    case ZWNR_GL_SAMPLER_PARAMETER_I:
      if (expected_size != sizeof(int32_t)) invalid_pname = true;
      break;
    case ZWNR_GL_SAMPLER_PARAMETER_FV:
    case ZWNR_GL_SAMPLER_PARAMETER_IV:
    case ZWNR_GL_SAMPLER_PARAMETER_IIV:
    case ZWNR_GL_SAMPLER_PARAMETER_IUIV:
      if (expected_size == 0) invalid_pname = true;
      if (expected_size != vector_param_size) unexpected_wl_array_size = true;
      break;
  }

  if (invalid_pname) {
    wl_resource_post_error(error_resource, ZWN_GLES_V32_ERROR_INVALID_ENUM,
        "pname %x is invalid", pname);
    return;
  }

  if (unexpected_wl_array_size) {
    wl_resource_post_error(error_resource, ZWN_COMPOSITOR_ERROR_WL_ARRAY_SIZE,
        "params must be %lu bytes, but got %lu bytes", expected_size,
        vector_param_size);
    return;
  }

  bool found = false;
  struct zwnr_gl_sampler_parameter *parameter;
  wl_array_for_each (parameter, &self->pending.parameters) {
    if (parameter->pname == pname) {
      found = true;
      break;
    }
  }

  if (!found) {
    parameter = wl_array_add(&self->pending.parameters, sizeof *parameter);
    parameter->pname = pname;
  }

  parameter->type = type;
  parameter->damaged = true;
  parameter->float_param = float_param;
  parameter->int_param = int_param;
  if (vector_param)
    memcpy(parameter->vector_param, vector_param->data, expected_size);
}

static void
zwnr_gl_sampler_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zwnr_gl_sampler_protocol_parameter_f(struct wl_client *client,
    struct wl_resource *resource, uint32_t pname,
    struct wl_array *param_wl_array)
{
  UNUSED(client);
  float param;
  struct zwnr_gl_sampler_impl *self = wl_resource_get_user_data(resource);

  if (zn_array_to_float(param_wl_array, &param) != 0) {
    wl_resource_post_error(resource, ZWN_COMPOSITOR_ERROR_WL_ARRAY_SIZE,
        "param is expected float (%lu bytes), but got %lu bytes", sizeof(float),
        param_wl_array->size);
    return;
  }

  zwnr_gl_sampler_set_parameter(
      self, ZWNR_GL_SAMPLER_PARAMETER_F, pname, param, 0, NULL, resource);
}

static void
zwnr_gl_sampler_protocol_parameter_i(struct wl_client *client,
    struct wl_resource *resource, uint32_t pname, int32_t param)
{
  UNUSED(client);
  struct zwnr_gl_sampler_impl *self = wl_resource_get_user_data(resource);

  zwnr_gl_sampler_set_parameter(
      self, ZWNR_GL_SAMPLER_PARAMETER_I, pname, 0, param, NULL, resource);
}

static void
zwnr_gl_sampler_protocol_parameter_fv(struct wl_client *client,
    struct wl_resource *resource, uint32_t pname, struct wl_array *params)
{
  UNUSED(client);
  struct zwnr_gl_sampler_impl *self = wl_resource_get_user_data(resource);

  zwnr_gl_sampler_set_parameter(
      self, ZWNR_GL_SAMPLER_PARAMETER_FV, pname, 0, 0, params, resource);
}

static void
zwnr_gl_sampler_protocol_parameter_iv(struct wl_client *client,
    struct wl_resource *resource, uint32_t pname, struct wl_array *params)
{
  UNUSED(client);
  struct zwnr_gl_sampler_impl *self = wl_resource_get_user_data(resource);

  zwnr_gl_sampler_set_parameter(
      self, ZWNR_GL_SAMPLER_PARAMETER_IV, pname, 0, 0, params, resource);
}

static void
zwnr_gl_sampler_protocol_parameter_iiv(struct wl_client *client,
    struct wl_resource *resource, uint32_t pname, struct wl_array *params)
{
  UNUSED(client);
  struct zwnr_gl_sampler_impl *self = wl_resource_get_user_data(resource);

  zwnr_gl_sampler_set_parameter(
      self, ZWNR_GL_SAMPLER_PARAMETER_IIV, pname, 0, 0, params, resource);
}

static void
zwnr_gl_sampler_protocol_parameter_iuiv(struct wl_client *client,
    struct wl_resource *resource, uint32_t pname, struct wl_array *params)
{
  UNUSED(client);
  struct zwnr_gl_sampler_impl *self = wl_resource_get_user_data(resource);

  zwnr_gl_sampler_set_parameter(
      self, ZWNR_GL_SAMPLER_PARAMETER_IUIV, pname, 0, 0, params, resource);
}

static const struct zwn_gl_sampler_interface implementation = {
    .destroy = zwnr_gl_sampler_protocol_destroy,
    .parameterf = zwnr_gl_sampler_protocol_parameter_f,
    .parameteri = zwnr_gl_sampler_protocol_parameter_i,
    .parameterfv = zwnr_gl_sampler_protocol_parameter_fv,
    .parameteriv = zwnr_gl_sampler_protocol_parameter_iv,
    .parameterIiv = zwnr_gl_sampler_protocol_parameter_iiv,
    .parameterIuiv = zwnr_gl_sampler_protocol_parameter_iuiv,
};

void
zwnr_gl_sampler_commit(struct zwnr_gl_sampler_impl *self)
{
  if (self->pending.parameters.size == 0) return;

  struct zwnr_gl_sampler_parameter *pending_parameter;
  wl_array_for_each (pending_parameter, &self->pending.parameters) {
    struct zwnr_gl_sampler_parameter *parameter;
    bool found = false;

    wl_array_for_each (parameter, &self->base.current.parameters) {
      if (parameter->pname == pending_parameter->pname) {
        found = true;
        break;
      }
    }

    if (!found) {
      parameter =
          wl_array_add(&self->base.current.parameters, sizeof *parameter);
    }

    memcpy(parameter, pending_parameter, sizeof *parameter);
  }

  self->pending.parameters.size = 0;
}

struct zwnr_gl_sampler_impl *
zwnr_gl_sampler_create(struct wl_client *client, uint32_t id)
{
  struct zwnr_gl_sampler_impl *self;
  struct wl_resource *resource;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  resource = wl_resource_create(client, &zwn_gl_sampler_interface, 1, id);
  if (resource == NULL) {
    zn_error("Failed to create a wl_resource");
    goto err_free;
  }

  wl_resource_set_implementation(
      resource, &implementation, self, zwnr_gl_sampler_handle_destroy);

  wl_array_init(&self->base.current.parameters);
  wl_array_init(&self->pending.parameters);

  wl_signal_init(&self->base.events.destroy);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zwnr_gl_sampler_destroy(struct zwnr_gl_sampler_impl *self)
{
  wl_signal_emit(&self->base.events.destroy, NULL);

  wl_list_remove(&self->base.events.destroy.listener_list);
  wl_array_release(&self->base.current.parameters);
  wl_array_release(&self->pending.parameters);
  free(self);
}
