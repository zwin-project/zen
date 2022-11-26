#include "gl-texture.h"

#include <GLES3/gl32.h>
#include <zen-common.h>
#include <zigen-gles-v32-protocol.h>
#include <zigen-protocol.h>

#include "zgnr/shm.h"

static void zgnr_gl_texture_destroy(struct zgnr_gl_texture_impl* self);

static size_t
gl_2d_image_size(GLenum format, GLenum type, GLsizei width, GLsizei height)
{
  size_t size = 0;
  switch (type) {
    case GL_UNSIGNED_BYTE:
      size = sizeof(GLubyte);
      break;
    case GL_BYTE:
      size = sizeof(GLbyte);
      break;
    case GL_UNSIGNED_SHORT:
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_5_5_5_1:
      size = sizeof(GLushort);
      break;
    case GL_SHORT:
      size = sizeof(GLshort);
      break;
    case GL_UNSIGNED_INT:
    case GL_UNSIGNED_INT_2_10_10_10_REV:
    case GL_UNSIGNED_INT_10F_11F_11F_REV:
    case GL_UNSIGNED_INT_5_9_9_9_REV:
    case GL_UNSIGNED_INT_24_8:
      size = sizeof(GLuint);
      break;
    case GL_INT:
      size = sizeof(GLint);
      break;
    case GL_HALF_FLOAT:
      size = sizeof(GLfloat) / 2;
      break;
    case GL_FLOAT:
    case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
      size = sizeof(GLfloat);
      break;
  }

  switch (format) {
    case GL_RED:
    case GL_RED_INTEGER:
    case GL_DEPTH_COMPONENT:
    case GL_ALPHA:
    case GL_STENCIL_INDEX:
    case GL_LUMINANCE:
      size *= 1;
      break;
    case GL_RG:
    case GL_RG_INTEGER:
    case GL_DEPTH_STENCIL:
    case GL_LUMINANCE_ALPHA:
      size *= 2;
      break;
    case GL_RGB:
    case GL_RGB_INTEGER:
      size *= 3;
      break;
    case GL_RGBA:
    case GL_RGBA_INTEGER:
      size *= 4;
      break;
  }

  return size * width * height;
}

static void
zgnr_gl_texture_handle_destroy(struct wl_resource* resource)
{
  struct zgnr_gl_texture_impl* self = wl_resource_get_user_data(resource);

  zgnr_gl_texture_destroy(self);
}

static void
zgnr_gl_texture_protocol_destroy(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);

  wl_resource_destroy(resource);
}

static void
zgnr_gl_texture_protocol_image_2d(struct wl_client* client,
    struct wl_resource* resource, uint32_t target, int32_t level,
    int32_t internal_format, uint32_t width, uint32_t height, int32_t border,
    uint32_t format, uint32_t type, struct wl_resource* data)
{
  UNUSED(client);
  struct zgnr_gl_texture_impl* self = wl_resource_get_user_data(resource);
  struct zgnr_shm_buffer* buffer = zgnr_shm_buffer_get(data);

  ssize_t expected_size = gl_2d_image_size(format, type, width, height);
  ssize_t actual_size = zgnr_shm_buffer_get_size(buffer);
  if (expected_size != actual_size) {
    wl_resource_post_error(resource, ZGN_GL_TEXTURE_ERROR_INVALID_IMAGE_SIZE,
        "Image size is expected to be %ld byte but got %ld byte", expected_size,
        actual_size);
    return;
  }

  zn_weak_resource_link(&self->pending.data, data);
  self->pending.target = target;
  self->pending.level = level;
  self->pending.internal_format = internal_format;
  self->pending.width = width;
  self->pending.height = height;
  self->pending.border = border;
  self->pending.format = format;
  self->pending.type = type;
}

const struct zgn_gl_texture_interface implementation = {
    .destroy = zgnr_gl_texture_protocol_destroy,
    .image_2d = zgnr_gl_texture_protocol_image_2d,
};

void
zgnr_gl_texture_commit(struct zgnr_gl_texture_impl* self)
{
  if (!self->pending.data.resource) return;  // nothing to commit

  struct zgnr_shm_buffer* buffer =
      zgnr_shm_buffer_get(self->pending.data.resource);

  if (self->base.current.data) {
    zgnr_mem_storage_unref(self->base.current.data);
    self->base.current.data = NULL;
  }

  void* data = zgnr_shm_buffer_get_data(buffer);
  ssize_t size = zgnr_shm_buffer_get_size(buffer);

  zgnr_shm_buffer_begin_access(buffer);
  self->base.current.data = zgnr_mem_storage_create(data, size);
  zgnr_shm_buffer_end_access(buffer);

  self->base.current.data_damaged = true;
  self->base.current.target = self->pending.target;
  self->base.current.level = self->pending.level;
  self->base.current.internal_format = self->pending.internal_format;
  self->base.current.width = self->pending.width;
  self->base.current.height = self->pending.height;
  self->base.current.border = self->pending.border;
  self->base.current.format = self->pending.format;
  self->base.current.type = self->pending.type;

  zgn_buffer_send_release(self->pending.data.resource);
  zn_weak_resource_unlink(&self->pending.data);
}

struct zgnr_gl_texture_impl*
zgnr_gl_texture_create(struct wl_client* client, uint32_t id)
{
  struct zgnr_gl_texture_impl* self;
  struct wl_resource* resource;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  resource = wl_resource_create(client, &zgn_gl_texture_interface, 1, id);
  if (resource == NULL) {
    zn_error("Failed to create a wl_resource");
    goto err_free;
  }

  wl_resource_set_implementation(
      resource, &implementation, self, zgnr_gl_texture_handle_destroy);
  wl_signal_init(&self->base.events.destroy);
  zn_weak_resource_init(&self->pending.data);
  self->base.current.data_damaged = false;
  self->base.current.data = NULL;

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zgnr_gl_texture_destroy(struct zgnr_gl_texture_impl* self)
{
  wl_signal_emit(&self->base.events.destroy, NULL);

  if (self->base.current.data) {
    zgnr_mem_storage_unref(self->base.current.data);
  }

  zn_weak_resource_unlink(&self->pending.data);

  wl_list_remove(&self->base.events.destroy.listener_list);
  free(self);
}
