#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

enum zwnr_gl_sampler_parameter_type {
  ZWNR_GL_SAMPLER_PARAMETER_F = 0,
  ZWNR_GL_SAMPLER_PARAMETER_I,
  ZWNR_GL_SAMPLER_PARAMETER_FV,
  ZWNR_GL_SAMPLER_PARAMETER_IV,
  ZWNR_GL_SAMPLER_PARAMETER_IIV,
  ZWNR_GL_SAMPLER_PARAMETER_IUIV,
};

struct zwnr_gl_sampler_parameter {
  uint32_t pname;
  enum zwnr_gl_sampler_parameter_type type;
  float float_param;
  int32_t int_param;
  uint8_t vector_param[16];

  // User may assign false to this; zwnr will only assign true to this.
  bool damaged;
};

struct zwnr_gl_sampler {
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
