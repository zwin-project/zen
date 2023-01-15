#pragma once

#include <wlr/render/wlr_renderer.h>

struct zn_wlr_egl;

struct wlr_renderer *wlr_glew_renderer_create_with_drm_fd(int drm_fd);
struct wlr_renderer *wlr_glew_renderer_create(struct zn_wlr_egl *egl);

struct zn_wlr_egl *wlr_glew_renderer_get_egl(struct wlr_renderer *renderer);

struct wlr_glew_texture_attribs {
  uint32_t target;
  uint32_t tex;

  bool has_alpha;
};

bool wlr_texture_is_glew(struct wlr_texture *texture);

void wlr_glew_texture_get_attribs(
    struct wlr_texture *texture, struct wlr_glew_texture_attribs *attribs);
