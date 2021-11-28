#include "opengl-texture.h"

#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>
#include <zigen-opengl-server-protocol.h>

static void zen_opengl_texture_destroy(struct zen_opengl_texture *texture);

static void
zen_opengl_texture_handle_destroy(struct wl_resource *resource)
{
  struct zen_opengl_texture *texture;

  texture = wl_resource_get_user_data(resource);

  zen_opengl_texture_destroy(texture);
}

static void
zen_opengl_texture_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zen_opengl_texture_protocol_atttach_2d(struct wl_client *client,
    struct wl_resource *resource, struct wl_resource *buffer)
{
  UNUSED(client);
  struct zen_opengl_texture *texture;

  texture = wl_resource_get_user_data(resource);

  zen_weak_link_set(&texture->pending.buffer_link, buffer);
}

static const struct zgn_opengl_texture_interface texture_interface = {
    .destroy = zen_opengl_texture_protocol_destroy,
    .attach_2d = zen_opengl_texture_protocol_atttach_2d,
};

WL_EXPORT void
zen_opengl_texture_commit(struct zen_opengl_texture *texture)
{
  struct wl_resource *buffer;
  struct wl_shm_buffer *shm_buffer;
  void *data;
  int32_t width, height;
  enum wl_shm_format format;

  buffer = texture->pending.buffer_link.resource;

  if (buffer == NULL) return;

  shm_buffer = wl_shm_buffer_get(buffer);
  data = wl_shm_buffer_get_data(shm_buffer);
  width = wl_shm_buffer_get_width(shm_buffer);
  height = wl_shm_buffer_get_height(shm_buffer);
  format = wl_shm_buffer_get_format(shm_buffer);

  wl_shm_buffer_begin_access(shm_buffer);

  glBindTexture(GL_TEXTURE_2D, texture->id);
  if (format == WL_SHM_FORMAT_ARGB8888 || format == WL_SHM_FORMAT_XRGB8888) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA,
        GL_UNSIGNED_INT_8_8_8_8_REV, data);
  }
  wl_shm_buffer_end_access(shm_buffer);
  wl_buffer_send_release(buffer);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  zen_weak_link_unset(&texture->pending.buffer_link);
}

WL_EXPORT struct zen_opengl_texture *
zen_opengl_texture_create(struct wl_client *client, uint32_t id)
{
  struct zen_opengl_texture *texture;
  struct wl_resource *resource;

  texture = zalloc(sizeof *texture);
  if (texture == NULL) {
    wl_client_post_no_memory(client);
    zen_log("opengl texture: failed to allocate memory\n");
    goto err;
  }

  resource = wl_resource_create(client, &zgn_opengl_texture_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    zen_log("opengl texture: failed to create a resource\n");
    goto err_resource;
  }

  wl_resource_set_implementation(
      resource, &texture_interface, texture, zen_opengl_texture_handle_destroy);

  glGenTextures(1, &texture->id);
  texture->resource = resource;
  zen_weak_link_init(&texture->pending.buffer_link);

  return texture;

err_resource:
  free(texture);

err:
  return NULL;
}

static void
zen_opengl_texture_destroy(struct zen_opengl_texture *texture)
{
  zen_weak_link_unset(&texture->pending.buffer_link);
  glDeleteTextures(1, &texture->id);
  free(texture);
}
