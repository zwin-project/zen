#include "opengl-vertex-buffer.h"

#include <libzen-compositor/libzen-compositor.h>
#include <zigen-opengl-server-protocol.h>

static void zen_opengl_vertex_buffer_destroy(
    struct zen_opengl_vertex_buffer* vertex_buffer);

static void
zen_opengl_vertex_buffer_handle_destroy(struct wl_resource* resource)
{
  struct zen_opengl_vertex_buffer* vertex_buffer;

  vertex_buffer = wl_resource_get_user_data(resource);

  zen_opengl_vertex_buffer_destroy(vertex_buffer);
}

static void
zen_opengl_vertex_buffer_protocol_destroy(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zen_opengl_vertex_buffer_protocol_attach(struct wl_client* client,
    struct wl_resource* resource, struct wl_resource* buffer)
{
  UNUSED(client);
  struct zen_opengl_vertex_buffer* vertex_buffer;

  vertex_buffer = wl_resource_get_user_data(resource);

  zen_weak_link_set(&vertex_buffer->pending.buffer_link, buffer);
}

static const struct zgn_opengl_vertex_buffer_interface vertex_buffer_interface =
    {
        .destroy = zen_opengl_vertex_buffer_protocol_destroy,
        .attach = zen_opengl_vertex_buffer_protocol_attach,
};

WL_EXPORT void
zen_opengl_vertex_buffer_commit(struct zen_opengl_vertex_buffer* vertex_buffer)
{
  struct wl_resource* buffer;
  struct wl_shm_buffer* shm_buffer;
  void* data;
  int32_t stride, height, buffer_size;

  buffer = vertex_buffer->pending.buffer_link.resource;

  if (buffer == NULL) return;

  shm_buffer = wl_shm_buffer_get(buffer);
  data = wl_shm_buffer_get_data(shm_buffer);
  stride = wl_shm_buffer_get_stride(shm_buffer);
  height = wl_shm_buffer_get_height(shm_buffer);
  buffer_size = stride * height;

  wl_shm_buffer_begin_access(shm_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer->id);
  glBufferData(GL_ARRAY_BUFFER, buffer_size, data, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  wl_shm_buffer_end_access(shm_buffer);

  wl_buffer_send_release(buffer);
  zen_weak_link_unset(&vertex_buffer->pending.buffer_link);
}

WL_EXPORT struct zen_opengl_vertex_buffer*
zen_opengl_vertex_buffer_create(struct wl_client* client, uint32_t id)
{
  struct zen_opengl_vertex_buffer* vertex_buffer;
  struct wl_resource* resource;

  vertex_buffer = zalloc(sizeof *vertex_buffer);
  if (vertex_buffer == NULL) {
    wl_client_post_no_memory(client);
    zen_log("opengl vertex buffer: failed to allocate memroy\n");
    goto err;
  }

  resource =
      wl_resource_create(client, &zgn_opengl_vertex_buffer_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    zen_log("opengl vertex buffer: failed to create a resource\n");
    goto err_resource;
  }

  wl_resource_set_implementation(resource, &vertex_buffer_interface,
      vertex_buffer, zen_opengl_vertex_buffer_handle_destroy);

  glGenBuffers(1, &vertex_buffer->id);
  vertex_buffer->resource = resource;
  zen_weak_link_init(&vertex_buffer->pending.buffer_link);

  return vertex_buffer;

err_resource:
  free(vertex_buffer);

err:
  return NULL;
}

static void
zen_opengl_vertex_buffer_destroy(struct zen_opengl_vertex_buffer* vertex_buffer)
{
  zen_weak_link_unset(&vertex_buffer->pending.buffer_link);
  glDeleteBuffers(1, &vertex_buffer->id);
  free(vertex_buffer);
}
