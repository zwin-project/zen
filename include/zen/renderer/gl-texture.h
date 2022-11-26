#pragma once

#include <zgnr/mem-storage.h>

#ifdef __cplusplus
extern "C" {
#endif

struct znr_gl_texture;

void znr_gl_texture_image_2d(struct znr_gl_texture *self, uint32_t target,
    int32_t level, int32_t internal_format, uint32_t width, uint32_t height,
    int32_t border, uint32_t format, uint32_t type,
    struct zgnr_mem_storage *storage);

struct znr_gl_texture *znr_gl_texture_create(
    struct znr_session *session, struct wl_display *display);

void znr_gl_texture_destroy(struct znr_gl_texture *self);

#ifdef __cplusplus
}
#endif
