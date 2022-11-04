#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_gles_v32 {
  struct {
    struct wl_signal new_rendering_unit;  // (struct zgnr_rendering_unit*)
  } events;
};

struct zgnr_gles_v32* zgnr_gles_v32_create(struct wl_display* display);

void zgnr_gles_v32_destroy(struct zgnr_gles_v32* self);

#ifdef __cplusplus
}
#endif
