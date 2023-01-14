#pragma once

#include <zwnr/gl-shader.h>

#include "system.h"
#include "zen/renderer/gl-shader.h"

struct zna_gl_shader {
  struct zwnr_gl_shader *zwnr_gl_shader;  // nonnull
  struct zna_system *system;

  /**
   * null before the initial commit or when the current session does not exists,
   * nonnull otherwise
   */
  struct znr_gl_shader *znr_gl_shader;

  struct wl_listener zwnr_gl_shader_destroy_listener;
  struct wl_listener session_destroyed_listener;
};

/**
 * Precondition:
 *  Current session exists && the zwnr_gl_shader has been committed
 */
void zna_gl_shader_apply_commit(struct zna_gl_shader *self, bool only_damaged);

struct zna_gl_shader *zna_gl_shader_create(
    struct zwnr_gl_shader *zwnr_gl_shader, struct zna_system *system);
