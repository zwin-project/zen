#pragma once

#include <zgnr/gl-base-technique.h>

#include "system.h"

struct zna_gl_base_technique {
  struct zgnr_gl_base_technique* zgnr_gl_base_technique;  // nonnull
  struct zna_system* system;                              // nonnull

  struct wl_listener zgnr_gl_base_technique_destroy_listener;
  struct wl_listener session_destroyed_listener;
};

/**
 * Precondition:
 *  Current session exists && the zgnr_gl_base_technique has been commited
 */
void zna_gl_base_technique_apply_commit(
    struct zna_gl_base_technique* self, bool only_damaged);

struct zna_gl_base_technique* zna_gl_base_technique_create(
    struct zgnr_gl_base_technique* zgnr_gl_base_technique,
    struct zna_system* system);
