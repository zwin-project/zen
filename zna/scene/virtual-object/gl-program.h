#pragma once

#include <zgnr/gl-program.h>

#include "system.h"
#include "zen/renderer/gl-program.h"

struct zna_gl_program {
  struct zgnr_gl_program *zgnr_gl_program;  // nonnull
  struct zna_system *system;                // nonnull

  /**
   * null before the initial commit or when the current session does not exists,
   * nonnull otherwise
   */
  struct znr_gl_program *znr_gl_program;

  struct wl_listener zgnr_gl_program_destroy_listener;
  struct wl_listener session_destroy_listener;
};

/**
 * Precondition:
 *  Current session exists && the zgnr_gl_program has been committed
 */
void zna_gl_program_apply_commit(
    struct zna_gl_program *self, bool only_damaged);

struct zna_gl_program *zna_gl_program_create(
    struct zgnr_gl_program *zgnr_gl_program, struct zna_system *system);
