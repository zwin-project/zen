#pragma once

#include <zwnr/gl-texture.h>

#include "system.h"
#include "zen/renderer/gl-texture.h"

struct zna_gl_texture {
  struct zwnr_gl_texture *zwnr_gl_texture;  // nonnull
  struct zna_system *system;

  /**
   * null before the initial commit or when the current session does not exists,
   * nonnull otherwise
   */
  struct znr_gl_texture *znr_gl_texture;

  struct wl_listener zwnr_gl_texture_destroy_listener;
  struct wl_listener session_destroyed_listener;
};

/**
 * Precondition:
 *  Current session exists && the zwnr_gl_texture has been committed
 */
void zna_gl_texture_apply_commit(
    struct zna_gl_texture *self, bool only_damaged);

struct zna_gl_texture *zna_gl_texture_create(
    struct zwnr_gl_texture *zwnr_gl_texture, struct zna_system *system);
