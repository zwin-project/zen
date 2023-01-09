#pragma once

#include <cairo-ft.h>
#include <stdbool.h>

enum zn_font_type {
  ZN_FONT_REGULAR = 0,
  ZN_FONT_BOLD,
  ZN_FONT_TYPE_COUNT,
};

/**
 * Call zn_font_init beforehand
 *
 * @return the reference to the cairo_font_face_t
 */
cairo_font_face_t *zn_font_face_get_cairo_font_face(enum zn_font_type type);

bool zn_font_init(void);

void zn_font_fini(void);
