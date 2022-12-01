#pragma once

#include <zgnr/mem-storage.h>

#include "zen/renderer/session.h"

#ifdef __cplusplus
extern "C" {
#endif

struct znr_gl_buffer;

void znr_gl_buffer_data(struct znr_gl_buffer *self, uint32_t target,
    struct zgnr_mem_storage *storage, uint32_t usage);

struct znr_gl_buffer *znr_gl_buffer_create(
    struct znr_session *session, struct wl_display *display);

void znr_gl_buffer_destroy(struct znr_gl_buffer *self);

#ifdef __cplusplus
}
#endif
