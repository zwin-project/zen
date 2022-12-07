#pragma once

#include <zgnr/gl-sampler.h>

#include "system.h"
#include "zen/renderer/gl-sampler.h"

struct zna_gl_sampler {
  struct zgnr_gl_sampler *zgnr_gl_sampler;  // nonnull
  struct zna_system *system;

  /**
   * null before the initial commit or when the current session does not exists,
   * nonnull otherwise
   */
  struct znr_gl_sampler *znr_gl_sampler;

  struct wl_listener zgnr_gl_sampler_destroy_listener;
  struct wl_listener session_destroyed_listener;
};

/**
 * Precondition:
 *  Current session exists && the zgnr_gl_sampler has been committed
 */
void zna_gl_sampler_apply_commit(
    struct zna_gl_sampler *self, bool only_damaged);

struct zna_gl_sampler *zna_gl_sampler_create(
    struct zgnr_gl_sampler *zgnr_gl_sampler, struct zna_system *system);
