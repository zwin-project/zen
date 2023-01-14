#pragma once

#include <zwnr/gl-vertex-array.h>

#include "system.h"
#include "zen/renderer/gl-vertex-array.h"

struct zna_gl_vertex_array {
  struct zwnr_gl_vertex_array *zwnr_gl_vertex_array;  // nonnull
  struct zna_system *system;                          // nonnull

  /**
   * null before the initial commit or when the current session does not exists,
   * nonnull otherwise
   */
  struct znr_gl_vertex_array *znr_gl_vertex_array;

  struct wl_listener zwnr_gl_vertex_array_destroy_listener;
  struct wl_listener session_destroy_listener;
};

/**
 * Precondition:
 *  Current session exists && the zwnr_gl_buffer has been committed
 */
void zna_gl_vertex_array_apply_commit(
    struct zna_gl_vertex_array *self, bool only_damage);

struct zna_gl_vertex_array *zna_gl_vertex_array_create(
    struct zwnr_gl_vertex_array *zwnr_gl_vertex_array,
    struct zna_system *system);
