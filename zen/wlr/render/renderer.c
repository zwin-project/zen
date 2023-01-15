#include <GL/glew.h>
#include <assert.h>
#include <drm_fourcc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wayland-util.h>
#include <wlr/render/interface.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>

#include "glew.h"
#include "zen/wlr/render/egl.h"

static const GLfloat verts[] = {
    1, 0,  // top right
    0, 0,  // top left
    1, 1,  // bottom right
    0, 1,  // bottom left
};

static const struct wlr_renderer_impl renderer_impl;

static bool
wlr_renderer_is_glew(struct wlr_renderer *wlr_renderer)
{
  return wlr_renderer->impl == &renderer_impl;
}

struct wlr_glew_renderer *
glew_get_renderer(struct wlr_renderer *wlr_renderer)
{
  assert(wlr_renderer_is_glew(wlr_renderer));
  return (struct wlr_glew_renderer *)wlr_renderer;
}

static struct wlr_glew_renderer *
glew_get_renderer_in_context(struct wlr_renderer *wlr_renderer)
{
  struct wlr_glew_renderer *renderer = glew_get_renderer(wlr_renderer);
  assert(zn_wlr_egl_is_current(renderer->egl));
  assert(renderer->current_buffer != NULL);
  return renderer;
}

static void
destroy_buffer(struct wlr_glew_buffer *buffer)
{
  wl_list_remove(&buffer->link);
  wlr_addon_finish(&buffer->addon);

  struct zn_wlr_egl_context prev_ctx;
  zn_wlr_egl_save_context(&prev_ctx);
  zn_wlr_egl_make_current(buffer->renderer->egl);

  push_glew_debug(buffer->renderer);

  glDeleteFramebuffers(1, &buffer->fbo);
  glDeleteRenderbuffers(1, &buffer->rbo);

  pop_glew_debug(buffer->renderer);

  zn_wlr_egl_destroy_image(buffer->renderer->egl, buffer->image);

  zn_wlr_egl_restore_context(&prev_ctx);

  free(buffer);
}

static void
handle_buffer_destroy(struct wlr_addon *addon)
{
  struct wlr_glew_buffer *buffer = wl_container_of(addon, buffer, addon);
  destroy_buffer(buffer);
}

static const struct wlr_addon_interface buffer_addon_impl = {
    .name = "wlr_glew_buffer",
    .destroy = handle_buffer_destroy,
};

static struct wlr_glew_buffer *
get_buffer(struct wlr_glew_renderer *renderer, struct wlr_buffer *wlr_buffer)
{
  struct wlr_addon *addon =
      wlr_addon_find(&wlr_buffer->addons, renderer, &buffer_addon_impl);
  if (addon == NULL) {
    return NULL;
  }
  struct wlr_glew_buffer *buffer = wl_container_of(addon, buffer, addon);
  return buffer;
}

static struct wlr_glew_buffer *
create_buffer(struct wlr_glew_renderer *renderer, struct wlr_buffer *wlr_buffer)
{
  struct wlr_glew_buffer *buffer = calloc(1, sizeof(*buffer));
  if (buffer == NULL) {
    wlr_log_errno(WLR_ERROR, "Allocation failed");
    return NULL;
  }
  buffer->buffer = wlr_buffer;
  buffer->renderer = renderer;

  struct wlr_dmabuf_attributes dmabuf = {0};
  if (!wlr_buffer_get_dmabuf(wlr_buffer, &dmabuf)) {
    goto error_buffer;
  }

  bool external_only;
  buffer->image = zn_wlr_egl_create_image_from_dmabuf(
      renderer->egl, &dmabuf, &external_only);
  if (buffer->image == EGL_NO_IMAGE_KHR) {
    goto error_buffer;
  }

  push_glew_debug(renderer);

  glGenRenderbuffers(1, &buffer->rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, buffer->rbo);
  glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, buffer->image);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glGenFramebuffers(1, &buffer->fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, buffer->fbo);
  glFramebufferRenderbuffer(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, buffer->rbo);
  GLenum fb_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  pop_glew_debug(renderer);

  if (fb_status != GL_FRAMEBUFFER_COMPLETE) {
    wlr_log(WLR_ERROR, "Failed to create FBO");
    goto error_image;
  }

  wlr_addon_init(
      &buffer->addon, &wlr_buffer->addons, renderer, &buffer_addon_impl);

  wl_list_insert(&renderer->buffers, &buffer->link);

  wlr_log(WLR_DEBUG, "Created GL FBO for buffer %dx%d", wlr_buffer->width,
      wlr_buffer->height);

  return buffer;

error_image:
  zn_wlr_egl_destroy_image(renderer->egl, buffer->image);
error_buffer:
  free(buffer);
  return NULL;
}

static bool
glew_bind_buffer(
    struct wlr_renderer *wlr_renderer, struct wlr_buffer *wlr_buffer)
{
  struct wlr_glew_renderer *renderer = glew_get_renderer(wlr_renderer);

  if (renderer->current_buffer != NULL) {
    assert(zn_wlr_egl_is_current(renderer->egl));

    push_glew_debug(renderer);
    glFlush();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    pop_glew_debug(renderer);

    wlr_buffer_unlock(renderer->current_buffer->buffer);
    renderer->current_buffer = NULL;
  }

  if (wlr_buffer == NULL) {
    zn_wlr_egl_unset_current(renderer->egl);
    return true;
  }

  zn_wlr_egl_make_current(renderer->egl);

  struct wlr_glew_buffer *buffer = get_buffer(renderer, wlr_buffer);
  if (buffer == NULL) {
    buffer = create_buffer(renderer, wlr_buffer);
  }
  if (buffer == NULL) {
    return false;
  }

  wlr_buffer_lock(wlr_buffer);
  renderer->current_buffer = buffer;

  push_glew_debug(renderer);
  glBindFramebuffer(GL_FRAMEBUFFER, renderer->current_buffer->fbo);
  pop_glew_debug(renderer);

  return true;
}

static void
glew_begin(struct wlr_renderer *wlr_renderer, uint32_t width, uint32_t height)
{
  struct wlr_glew_renderer *renderer =
      glew_get_renderer_in_context(wlr_renderer);

  push_glew_debug(renderer);

  glViewport(0, 0, width, height);
  renderer->viewport_width = width;
  renderer->viewport_height = height;

  // refresh projection matrix
  wlr_matrix_projection(
      renderer->projection, width, height, WL_OUTPUT_TRANSFORM_NORMAL);

  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  // XXX: maybe we should save output projection and remove some of the need
  // for users to sling matricies themselves

  pop_glew_debug(renderer);
}

static void
glew_end(struct wlr_renderer *wlr_renderer)
{
  glew_get_renderer_in_context(wlr_renderer);
  // no-op
}

static void
glew_clear(struct wlr_renderer *wlr_renderer, const float color[static 4])
{
  struct wlr_glew_renderer *renderer =
      glew_get_renderer_in_context(wlr_renderer);

  push_glew_debug(renderer);
  glClearColor(color[0], color[1], color[2], color[3]);
  glClear(GL_COLOR_BUFFER_BIT);
  pop_glew_debug(renderer);
}

static void
glew_scissor(struct wlr_renderer *wlr_renderer, struct wlr_box *box)
{
  struct wlr_glew_renderer *renderer =
      glew_get_renderer_in_context(wlr_renderer);

  push_glew_debug(renderer);
  if (box != NULL) {
    glScissor(box->x, box->y, box->width, box->height);
    glEnable(GL_SCISSOR_TEST);
  } else {
    glDisable(GL_SCISSOR_TEST);
  }
  pop_glew_debug(renderer);
}

static const float flip_180[9] = {
    1.0f, 0.0f, 0.0f,   //
    0.0f, -1.0f, 0.0f,  //
    0.0f, 0.0f, 1.0f,   //
};

static bool
glew_render_subtexture_with_matrix(struct wlr_renderer *wlr_renderer,
    struct wlr_texture *wlr_texture, const struct wlr_fbox *box,
    const float matrix[static 9], float alpha)
{
  struct wlr_glew_renderer *renderer =
      glew_get_renderer_in_context(wlr_renderer);
  struct wlr_glew_texture *texture = glew_get_texture(wlr_texture);
  assert(texture->renderer == renderer);

  struct wlr_glew_tex_shader *shader = NULL;

  switch (texture->target) {
    case GL_TEXTURE_2D:
      if (texture->has_alpha) {
        shader = &renderer->shaders.tex_rgba;
      } else {
        shader = &renderer->shaders.tex_rgbx;
      }
      break;
    case GL_TEXTURE_EXTERNAL_OES:
      shader = &renderer->shaders.tex_ext;

      if (!GLEW_OES_EGL_image_external) {
        wlr_log(WLR_ERROR,
            "Failed to render texture: "
            "GL_TEXTURE_EXTERNAL_OES not supported");
        return false;
      }
      break;
    default:
      abort();
  }

  float gl_matrix[9];
  wlr_matrix_multiply(gl_matrix, renderer->projection, matrix);
  wlr_matrix_multiply(gl_matrix, flip_180, gl_matrix);

  // OpenGL ES 2 requires the glUniformMatrix3fv transpose parameter to be
  // set to GL_FALSE
  wlr_matrix_transpose(gl_matrix, gl_matrix);

  push_glew_debug(renderer);

  if (!texture->has_alpha && alpha == 1.0) {
    glDisable(GL_BLEND);
  } else {
    glEnable(GL_BLEND);
  }

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(texture->target, texture->tex);

  glTexParameteri(texture->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glUseProgram(shader->program);

  glUniformMatrix3fv(shader->proj, 1, GL_FALSE, gl_matrix);
  glUniform1i(shader->tex, 0);
  glUniform1f(shader->alpha, alpha);

  const GLfloat x1 = box->x / wlr_texture->width;
  const GLfloat y1 = box->y / wlr_texture->height;
  const GLfloat x2 = (box->x + box->width) / wlr_texture->width;
  const GLfloat y2 = (box->y + box->height) / wlr_texture->height;
  const GLfloat texcoord[] = {
      x2, y1,  // top right
      x1, y1,  // top left
      x2, y2,  // bottom right
      x1, y2,  // bottom left
  };

  glVertexAttribPointer(shader->pos_attrib, 2, GL_FLOAT, GL_FALSE, 0, verts);
  glVertexAttribPointer(shader->tex_attrib, 2, GL_FLOAT, GL_FALSE, 0, texcoord);

  glEnableVertexAttribArray(shader->pos_attrib);
  glEnableVertexAttribArray(shader->tex_attrib);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glDisableVertexAttribArray(shader->pos_attrib);
  glDisableVertexAttribArray(shader->tex_attrib);

  glBindTexture(texture->target, 0);

  pop_glew_debug(renderer);
  return true;
}

static void
glew_render_quad_with_matrix(struct wlr_renderer *wlr_renderer,
    const float color[static 4], const float matrix[static 9])
{
  struct wlr_glew_renderer *renderer =
      glew_get_renderer_in_context(wlr_renderer);

  float gl_matrix[9];
  wlr_matrix_multiply(gl_matrix, renderer->projection, matrix);
  wlr_matrix_multiply(gl_matrix, flip_180, gl_matrix);

  push_glew_debug(renderer);

  if (color[3] == 1.0) {
    glDisable(GL_BLEND);
  } else {
    glEnable(GL_BLEND);
  }

  glUseProgram(renderer->shaders.quad.program);

  glUniformMatrix3fv(renderer->shaders.quad.proj, 1, GL_TRUE, gl_matrix);
  glUniform4f(
      renderer->shaders.quad.color, color[0], color[1], color[2], color[3]);

  glVertexAttribPointer(
      renderer->shaders.quad.pos_attrib, 2, GL_FLOAT, GL_FALSE, 0, verts);

  glEnableVertexAttribArray(renderer->shaders.quad.pos_attrib);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glDisableVertexAttribArray(renderer->shaders.quad.pos_attrib);

  pop_glew_debug(renderer);
}

static const uint32_t *
glew_get_shm_texture_formats(struct wlr_renderer *wlr_renderer, size_t *len)
{
  struct wlr_glew_renderer *renderer = glew_get_renderer(wlr_renderer);
  return get_glew_shm_formats(renderer, len);
}

static const struct wlr_drm_format_set *
glew_get_dmabuf_texture_formats(struct wlr_renderer *wlr_renderer)
{
  struct wlr_glew_renderer *renderer = glew_get_renderer(wlr_renderer);
  return zn_wlr_egl_get_dmabuf_texture_formats(renderer->egl);
}

static const struct wlr_drm_format_set *
glew_get_render_formats(struct wlr_renderer *wlr_renderer)
{
  struct wlr_glew_renderer *renderer = glew_get_renderer(wlr_renderer);
  return zn_wlr_egl_get_dmabuf_render_formats(renderer->egl);
}

static uint32_t
glew_preferred_read_format(struct wlr_renderer *wlr_renderer)
{
  struct wlr_glew_renderer *renderer =
      glew_get_renderer_in_context(wlr_renderer);

  push_glew_debug(renderer);

  GLint gl_format = -1, gl_type = -1;
  glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &gl_format);
  glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &gl_type);

  EGLint alpha_size = -1;
  glBindRenderbuffer(GL_RENDERBUFFER, renderer->current_buffer->rbo);
  glGetRenderbufferParameteriv(
      GL_RENDERBUFFER, GL_RENDERBUFFER_ALPHA_SIZE, &alpha_size);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  pop_glew_debug(renderer);

  const struct wlr_glew_pixel_format *fmt =
      get_glew_format_from_gl(gl_format, gl_type, alpha_size > 0);
  if (fmt != NULL) {
    return fmt->drm_format;
  }

  if (GLEW_EXT_read_format_bgra) {
    return DRM_FORMAT_XRGB8888;
  }
  return DRM_FORMAT_XBGR8888;
}

static bool
glew_read_pixels(struct wlr_renderer *wlr_renderer, uint32_t drm_format,
    uint32_t *flags, uint32_t stride, uint32_t width, uint32_t height,
    uint32_t src_x, uint32_t src_y, uint32_t dst_x, uint32_t dst_y, void *data)
{
  struct wlr_glew_renderer *renderer =
      glew_get_renderer_in_context(wlr_renderer);

  const struct wlr_glew_pixel_format *fmt =
      get_glew_format_from_drm(drm_format);
  if (fmt == NULL || !is_glew_pixel_format_supported(renderer, fmt)) {
    wlr_log(WLR_ERROR,
        "Cannot read pixels: unsupported pixel format 0x%" PRIX32, drm_format);
    return false;
  }

  if (fmt->gl_format == GL_BGRA_EXT && !GLEW_EXT_read_format_bgra) {
    wlr_log(WLR_ERROR,
        "Cannot read pixels: missing GL_EXT_read_format_bgra extension");
    return false;
  }

  const struct wlr_glew_drm_pixel_format *drm_fmt =
      get_glew_drm_format_from_drm(fmt->drm_format);
  assert(drm_fmt);

  push_glew_debug(renderer);

  // Make sure any pending drawing is finished before we try to read it
  glFinish();

  glGetError();  // Clear the error flag

  unsigned char *p = (unsigned char *)data + dst_y * stride;
  uint32_t pack_stride = width * drm_fmt->bpp / 8;
  if (pack_stride == stride && dst_x == 0) {
    // Under these particular conditions, we can read the pixels with only
    // one glReadPixels call

    glReadPixels(src_x, src_y, width, height, fmt->gl_format, fmt->gl_type, p);
  } else {
    // Unfortunately GLES2 doesn't support GL_PACK_*, so we have to read
    // the lines out row by row
    for (size_t i = 0; i < height; ++i) {
      uint32_t y = src_y + i;
      glReadPixels(src_x, y, width, 1, fmt->gl_format, fmt->gl_type,
          p + i * stride + dst_x * drm_fmt->bpp / 8);
    }
  }

  pop_glew_debug(renderer);

  if (flags != NULL) {
    *flags = 0;
  }

  return glGetError() == GL_NO_ERROR;
}

static int
glew_get_drm_fd(struct wlr_renderer *wlr_renderer)
{
  struct wlr_glew_renderer *renderer = glew_get_renderer(wlr_renderer);

  if (renderer->drm_fd < 0) {
    renderer->drm_fd = zn_wlr_egl_dup_drm_fd(renderer->egl);
  }

  return renderer->drm_fd;
}

static uint32_t
glew_get_render_buffer_caps(struct wlr_renderer *renderer)
{
  (void)renderer;
  return WLR_BUFFER_CAP_DMABUF;
}

WL_EXPORT struct zn_wlr_egl *
wlr_glew_renderer_get_egl(struct wlr_renderer *wlr_renderer)
{
  struct wlr_glew_renderer *renderer = glew_get_renderer(wlr_renderer);
  return renderer->egl;
}

static void
glew_destroy(struct wlr_renderer *wlr_renderer)
{
  struct wlr_glew_renderer *renderer = glew_get_renderer(wlr_renderer);

  zn_wlr_egl_make_current(renderer->egl);

  struct wlr_glew_buffer *buffer, *buffer_tmp;
  wl_list_for_each_safe (buffer, buffer_tmp, &renderer->buffers, link) {
    destroy_buffer(buffer);
  }

  struct wlr_glew_texture *tex, *tex_tmp;
  wl_list_for_each_safe (tex, tex_tmp, &renderer->textures, link) {
    glew_texture_destroy(tex);
  }

  push_glew_debug(renderer);
  glDeleteProgram(renderer->shaders.quad.program);
  glDeleteProgram(renderer->shaders.tex_rgba.program);
  glDeleteProgram(renderer->shaders.tex_rgbx.program);
  glDeleteProgram(renderer->shaders.tex_ext.program);
  pop_glew_debug(renderer);

  if (GLEW_KHR_debug) {
    glDisable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(NULL, NULL);
  }

  zn_wlr_egl_unset_current(renderer->egl);
  zn_wlr_egl_destroy(renderer->egl);

  if (renderer->drm_fd >= 0) {
    close(renderer->drm_fd);
  }

  free(renderer);
}

static const struct wlr_renderer_impl renderer_impl = {
    .destroy = glew_destroy,
    .bind_buffer = glew_bind_buffer,
    .begin = glew_begin,
    .end = glew_end,
    .clear = glew_clear,
    .scissor = glew_scissor,
    .render_subtexture_with_matrix = glew_render_subtexture_with_matrix,
    .render_quad_with_matrix = glew_render_quad_with_matrix,
    .get_shm_texture_formats = glew_get_shm_texture_formats,
    .get_dmabuf_texture_formats = glew_get_dmabuf_texture_formats,
    .get_render_formats = glew_get_render_formats,
    .preferred_read_format = glew_preferred_read_format,
    .read_pixels = glew_read_pixels,
    .get_drm_fd = glew_get_drm_fd,
    .get_render_buffer_caps = glew_get_render_buffer_caps,
    .texture_from_buffer = glew_texture_from_buffer,
};

static enum wlr_log_importance
glew_log_importance_to_wlr(GLenum type)
{
  switch (type) {
    case GL_DEBUG_TYPE_ERROR:  // fallthrough
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      return WLR_ERROR;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:  // fallthrough
    case GL_DEBUG_TYPE_PORTABILITY:          // fallthrough
    case GL_DEBUG_TYPE_PERFORMANCE:          // fallthrough
    case GL_DEBUG_TYPE_OTHER:                // fallthrough
    case GL_DEBUG_TYPE_MARKER:               // fallthrough
    case GL_DEBUG_TYPE_PUSH_GROUP:           // fallthrough
    case GL_DEBUG_TYPE_POP_GROUP:
      return WLR_DEBUG;
    default:
      return WLR_DEBUG;
  }
}

void
push_glew_debug_(
    struct wlr_glew_renderer *renderer, const char *file, const char *func)
{
  (void)renderer;
  if (!GLEW_KHR_debug) {
    return;
  }

  int len = snprintf(NULL, 0, "%s:%s", file, func) + 1;
  char str[len];
  snprintf(str, len, "%s:%s", file, func);
  glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, str);
}

void
pop_glew_debug(struct wlr_glew_renderer *renderer)
{
  (void)renderer;
  if (GLEW_KHR_debug) {
    glPopDebugGroup();
  }
}

static void
glew_log(GLenum src, GLenum type, GLuint id, GLenum severity, GLsizei len,
    const GLchar *msg, const void *user)
{
  (void)src;
  (void)id;
  (void)severity;
  (void)len;
  (void)user;
  _wlr_log(glew_log_importance_to_wlr(type), "[GLEW] %s", msg);
}

static GLuint
compile_shader(
    struct wlr_glew_renderer *renderer, GLuint type, const GLchar *src)
{
  push_glew_debug(renderer);

  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);

  GLint ok;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
  if (ok == GL_FALSE) {
    glDeleteShader(shader);
    shader = 0;
  }

  pop_glew_debug(renderer);
  return shader;
}

static GLuint
link_program(struct wlr_glew_renderer *renderer, const GLchar *vert_src,
    const GLchar *frag_src)
{
  push_glew_debug(renderer);

  GLuint vert = compile_shader(renderer, GL_VERTEX_SHADER, vert_src);
  if (!vert) {
    goto error;
  }

  GLuint frag = compile_shader(renderer, GL_FRAGMENT_SHADER, frag_src);
  if (!frag) {
    glDeleteShader(vert);
    goto error;
  }

  GLuint prog = glCreateProgram();
  glAttachShader(prog, vert);
  glAttachShader(prog, frag);
  glLinkProgram(prog);

  glDetachShader(prog, vert);
  glDetachShader(prog, frag);
  glDeleteShader(vert);
  glDeleteShader(frag);

  GLint ok;
  glGetProgramiv(prog, GL_LINK_STATUS, &ok);
  if (ok == GL_FALSE) {
    glDeleteProgram(prog);
    goto error;
  }

  pop_glew_debug(renderer);
  return prog;

error:
  pop_glew_debug(renderer);
  return 0;
}

extern const GLchar quad_vertex_src[];
extern const GLchar quad_fragment_src[];
extern const GLchar tex_vertex_src[];
extern const GLchar tex_fragment_src_rgba[];
extern const GLchar tex_fragment_src_rgbx[];
extern const GLchar tex_fragment_src_external[];

WL_EXPORT struct wlr_renderer *
wlr_glew_renderer_create_with_drm_fd(int drm_fd)
{
  struct zn_wlr_egl *egl = zn_wlr_egl_create_with_drm_fd(drm_fd);
  if (egl == NULL) {
    wlr_log(WLR_ERROR, "Could not initialize EGL");
    return NULL;
  }

  struct wlr_renderer *renderer = wlr_glew_renderer_create(egl);
  if (!renderer) {
    wlr_log(WLR_ERROR, "Failed to create GLEW renderer");
    zn_wlr_egl_destroy(egl);
    return NULL;
  }

  return renderer;
}

WL_EXPORT struct wlr_renderer *
wlr_glew_renderer_create(struct zn_wlr_egl *egl)
{
  if (!zn_wlr_egl_make_current(egl)) {
    return NULL;
  }

  GLenum err = glewInit();
  if (GLEW_OK != err) {
    wlr_log(
        WLR_ERROR, "Failed to initialize GLEW: %s", glewGetErrorString(err));
    return NULL;
  }

  struct wlr_glew_renderer *renderer =
      calloc(1, sizeof(struct wlr_glew_renderer));
  if (renderer == NULL) {
    return NULL;
  }
  wlr_renderer_init(&renderer->wlr_renderer, &renderer_impl);

  wl_list_init(&renderer->buffers);
  wl_list_init(&renderer->textures);

  renderer->egl = egl;
  renderer->drm_fd = -1;

  wlr_log(WLR_INFO, "Creating GLEW(%s) renderer", glewGetString(GLEW_VERSION));
  wlr_log(WLR_INFO, "Using %s", glGetString(GL_VERSION));
  wlr_log(WLR_INFO, "GL vendor: %s", glGetString(GL_VENDOR));
  wlr_log(WLR_INFO, "GL renderer: %s", glGetString(GL_RENDERER));

  if (!renderer->egl->exts.EXT_image_dma_buf_import) {
    wlr_log(WLR_ERROR, "EGL_EXT_image_dma_buf_import not supported");
    free(renderer);
    return NULL;
  }
  if (!GLEW_EXT_texture_format_BGRA8888) {
    wlr_log(WLR_ERROR, "BGRA8888 format not supported");
    free(renderer);
    return NULL;
  }
  if (!GLEW_EXT_unpack_subimage) {
    wlr_log(WLR_ERROR, "GL_EXT_unpack_subimage not supported");
    free(renderer);
    return NULL;
  }

  if (GLEW_KHR_debug) {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glew_log, NULL);

    // Silence unwanted message types
    glDebugMessageControl(
        GL_DONT_CARE, GL_DEBUG_TYPE_POP_GROUP, GL_DONT_CARE, 0, NULL, GL_FALSE);
    glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_PUSH_GROUP, GL_DONT_CARE,
        0, NULL, GL_FALSE);
  }

  push_glew_debug(renderer);

  GLuint prog;
  renderer->shaders.quad.program = prog =
      link_program(renderer, quad_vertex_src, quad_fragment_src);
  if (!renderer->shaders.quad.program) {
    goto error;
  }
  renderer->shaders.quad.proj = glGetUniformLocation(prog, "proj");
  renderer->shaders.quad.color = glGetUniformLocation(prog, "color");
  renderer->shaders.quad.pos_attrib = glGetAttribLocation(prog, "pos");

  renderer->shaders.tex_rgba.program = prog =
      link_program(renderer, tex_vertex_src, tex_fragment_src_rgba);
  if (!renderer->shaders.tex_rgba.program) {
    goto error;
  }
  renderer->shaders.tex_rgba.proj = glGetUniformLocation(prog, "proj");
  renderer->shaders.tex_rgba.tex = glGetUniformLocation(prog, "tex");
  renderer->shaders.tex_rgba.alpha = glGetUniformLocation(prog, "alpha");
  renderer->shaders.tex_rgba.pos_attrib = glGetAttribLocation(prog, "pos");
  renderer->shaders.tex_rgba.tex_attrib = glGetAttribLocation(prog, "texcoord");

  renderer->shaders.tex_rgbx.program = prog =
      link_program(renderer, tex_vertex_src, tex_fragment_src_rgbx);
  if (!renderer->shaders.tex_rgbx.program) {
    goto error;
  }
  renderer->shaders.tex_rgbx.proj = glGetUniformLocation(prog, "proj");
  renderer->shaders.tex_rgbx.tex = glGetUniformLocation(prog, "tex");
  renderer->shaders.tex_rgbx.alpha = glGetUniformLocation(prog, "alpha");
  renderer->shaders.tex_rgbx.pos_attrib = glGetAttribLocation(prog, "pos");
  renderer->shaders.tex_rgbx.tex_attrib = glGetAttribLocation(prog, "texcoord");

  if (GLEW_OES_EGL_image_external) {
    renderer->shaders.tex_ext.program = prog =
        link_program(renderer, tex_vertex_src, tex_fragment_src_external);
    if (!renderer->shaders.tex_ext.program) {
      goto error;
    }
    renderer->shaders.tex_ext.proj = glGetUniformLocation(prog, "proj");
    renderer->shaders.tex_ext.tex = glGetUniformLocation(prog, "tex");
    renderer->shaders.tex_ext.alpha = glGetUniformLocation(prog, "alpha");
    renderer->shaders.tex_ext.pos_attrib = glGetAttribLocation(prog, "pos");
    renderer->shaders.tex_ext.tex_attrib =
        glGetAttribLocation(prog, "texcoord");
  }

  pop_glew_debug(renderer);

  zn_wlr_egl_unset_current(renderer->egl);

  return &renderer->wlr_renderer;

error:
  glDeleteProgram(renderer->shaders.quad.program);
  glDeleteProgram(renderer->shaders.tex_rgba.program);
  glDeleteProgram(renderer->shaders.tex_rgbx.program);
  glDeleteProgram(renderer->shaders.tex_ext.program);

  pop_glew_debug(renderer);

  if (GL_KHR_debug) {
    glDisable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(NULL, NULL);
  }

  zn_wlr_egl_unset_current(renderer->egl);

  free(renderer);
  return NULL;
}
