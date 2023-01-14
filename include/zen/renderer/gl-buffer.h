#pragma once

#include <zwnr/mem-storage.h>

#include "zen/renderer/dispatcher.h"

#ifdef __cplusplus
extern "C" {
#endif

struct znr_gl_buffer;

void znr_gl_buffer_data(struct znr_gl_buffer *self, uint32_t target,
    struct zwnr_mem_storage *storage, uint32_t usage);

struct znr_gl_buffer *znr_gl_buffer_create(
    struct znr_dispatcher *dispatcher, struct wl_display *display);

void znr_gl_buffer_destroy(struct znr_gl_buffer *self);

#ifdef __cplusplus
}
#endif
