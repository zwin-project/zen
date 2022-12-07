#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

enum zgnr_gl_sampler_parameter_type {
  ZGNR_GL_SAMPLER_PARAMETER_F = 0,
  ZGNR_GL_SAMPLER_PARAMETER_I,
  ZGNR_GL_SAMPLER_PARAMETER_FV,
  ZGNR_GL_SAMPLER_PARAMETER_IV,
  ZGNR_GL_SAMPLER_PARAMETER_IIV,
  ZGNR_GL_SAMPLER_PARAMETER_IUIV,
};

struct zgnr_gl_sampler_parameter {
  uint32_t pname;
  enum zgnr_gl_sampler_parameter_type type;
  float float_param;
  int32_t int_param;
  uint8_t vector_param[16];

  // User may assign false to this; zgnr will only assign true to this.
  bool damaged;
};

struct zgnr_gl_sampler {
  struct {
    struct wl_signal destroy;
  } events;

  struct {
    struct wl_array parameters;
  } current;

  void *user_data;
};

#ifdef __cplusplus
}
#endif
