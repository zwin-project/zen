#pragma once

#include "zen/renderer/session.h"

#ifdef __cplusplus
extern "C" {
#endif

struct znr_gl_sampler;

void znr_gl_sampler_parameter_f(
    struct znr_gl_sampler *self, uint32_t pname, float param);

void znr_gl_sampler_parameter_i(
    struct znr_gl_sampler *self, uint32_t pname, int32_t param);

void znr_gl_sampler_parameter_fv(
    struct znr_gl_sampler *self, uint32_t pname, float *params);

void znr_gl_sampler_parameter_iv(
    struct znr_gl_sampler *self, uint32_t pname, int32_t *params);

void znr_gl_sampler_parameter_iiv(
    struct znr_gl_sampler *self, uint32_t pname, int32_t *params);

void znr_gl_sampler_parameter_iuiv(
    struct znr_gl_sampler *self, uint32_t pname, uint32_t *params);

struct znr_gl_sampler *znr_gl_sampler_create(struct znr_session *session);

void znr_gl_sampler_destroy(struct znr_gl_sampler *self);

#ifdef __cplusplus
}
#endif
