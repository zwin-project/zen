#pragma once

#include <zwnr/gl-buffer.h>

#include "system.h"
#include "zen/renderer/gl-buffer.h"

struct zna_gl_buffer {
  struct zwnr_gl_buffer *zwnr_gl_buffer;  // nonnull
  struct zna_system *system;              // nonnull

  /**
   * null before the initial commit or when the current session does not exists,
   * nonnull otherwise
   */
  struct znr_gl_buffer *znr_gl_buffer;

  struct wl_listener zwnr_gl_buffer_destroy_listener;
  struct wl_listener session_destroyed_listener;
};

/**
 * Precondition:
 *  Current session exists && the zwnr_gl_buffer has been committed
 */
void zna_gl_buffer_apply_commit(struct zna_gl_buffer *self, bool only_damaged);

struct zna_gl_buffer *zna_gl_buffer_create(
    struct zwnr_gl_buffer *zwnr_gl_buffer, struct zna_system *system);
