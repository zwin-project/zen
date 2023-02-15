#pragma once

#include <GL/glew.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/util/addon.h>
#include <wlr/util/log.h>

#include "zen/wlr/render/egl.h"
#include "zen/wlr/render/glew.h"

struct wlr_glew_pixel_format {
  uint32_t drm_format;
  GLint gl_format, gl_type;
  bool has_alpha;
};

struct wlr_glew_drm_pixel_format {
  uint32_t drm_format;

  /* Equivalent of the format if it has an alpha channel,
   * DRM_FORMAT_INVALID (0) if NA
   */
  uint32_t opaque_substitute;

  /* Bits per pixels */
  uint32_t bpp;

  /* True if the format has an alpha channel */
  bool has_alpha;
};

struct wlr_glew_tex_shader {
  GLuint program;
  GLint proj;
  GLint tex;
  GLint alpha;
  GLint pos_attrib;
  GLint tex_attrib;
};

struct wlr_glew_renderer {
  struct wlr_renderer wlr_renderer;

  float projection[9];
  struct zn_wlr_egl *egl;
  int drm_fd;

  struct {
    struct {
      GLuint program;
      GLint proj;
      GLint color;
      GLint pos_attrib;
    } quad;
    struct wlr_glew_tex_shader tex_rgba;
    struct wlr_glew_tex_shader tex_rgbx;
    struct wlr_glew_tex_shader tex_ext;
  } shaders;

  struct wl_list buffers;   // wlr_glew_buffer.link
  struct wl_list textures;  // wlr_glew_texture.link

  struct wlr_glew_buffer *current_buffer;
  uint32_t viewport_width, viewport_height;
};

struct wlr_glew_buffer {
  struct wlr_buffer *buffer;
  struct wlr_glew_renderer *renderer;
  struct wl_list link;  // wlr_glew_renderer.buffers

  EGLImageKHR image;
  GLuint rbo;
  GLuint fbo;

  struct wlr_addon addon;
};

struct wlr_glew_texture {
  struct wlr_texture wlr_texture;
  struct wlr_glew_renderer *renderer;
  struct wl_list link;  // wlr_glew_renderer.textures

  // Basically:
  //   GL_TEXTURE_2D == mutable
  //   GL_TEXTURE_EXTERNAL_OES == immutable
  GLenum target;
  GLuint tex;

  EGLImageKHR image;

  bool has_alpha;

  // Only affects target == GL_TEXTURE_2D
  uint32_t drm_format;  // used to interpret upload data
  // If imported from a wlr_buffer
  struct wlr_buffer *buffer;
  struct wlr_addon buffer_addon;
};

bool is_glew_pixel_format_supported(const struct wlr_glew_renderer *renderer,
    const struct wlr_glew_pixel_format *format);

const struct wlr_glew_drm_pixel_format *get_glew_drm_format_from_drm(
    uint32_t fmt);

const struct wlr_glew_pixel_format *get_glew_format_from_drm(uint32_t fmt);

const struct wlr_glew_pixel_format *get_glew_format_from_gl(
    GLint gl_format, GLint gl_type, bool alpha);

const uint32_t *get_glew_shm_formats(
    const struct wlr_glew_renderer *renderer, size_t *len);

struct wlr_glew_renderer *glew_get_renderer(struct wlr_renderer *wlr_renderer);

struct wlr_glew_texture *glew_get_texture(struct wlr_texture *wlr_texture);

struct wlr_texture *glew_texture_from_wl_drm(
    struct wlr_renderer *wlr_renderer, struct wl_resource *data);

struct wlr_texture *glew_texture_from_buffer(
    struct wlr_renderer *wlr_renderer, struct wlr_buffer *buffer);

void glew_texture_destroy(struct wlr_glew_texture *texture);

void push_glew_debug_(
    struct wlr_glew_renderer *renderer, const char *file, const char *func);

#define push_glew_debug(renderer) \
  push_glew_debug_(renderer, _WLR_FILENAME, __func__)

void pop_glew_debug(struct wlr_glew_renderer *renderer);
