#include "gl-vertex-array.h"

#include <zen-common.h>
#include <zwin-gles-v32-protocol.h>
#include <zwin-protocol.h>

#include "gl-buffer.h"
#include "gl-vertex-attrib.h"

static void zwnr_gl_vertex_array_destroy(
    struct zwnr_gl_vertex_array_impl *self);

static void
zwnr_gl_vertex_array_handle_destroy(struct wl_resource *resource)
{
  struct zwnr_gl_vertex_array_impl *self = wl_resource_get_user_data(resource);
  zwnr_gl_vertex_array_destroy(self);
}

static void
zwnr_gl_vertex_array_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static struct zwnr_gl_vertex_attrib *
zwnr_gl_vertex_array_ensure_vertex_attrib(
    struct zwnr_gl_vertex_array_impl *self, uint32_t index)
{
  struct zwnr_gl_vertex_attrib *attrib;
  wl_list_for_each (attrib, &self->pending.vertex_attrib_list, link) {
    if (attrib->index == index) {
      return attrib;
    }
  }

  attrib = zwnr_gl_vertex_attrib_create(index);
  wl_list_insert(&self->pending.vertex_attrib_list, &attrib->link);

  return attrib;
}

static void
zwnr_gl_vertex_array_protocol_enable_vertex_attrib_array(
    struct wl_client *client, struct wl_resource *resource, uint32_t index)
{
  UNUSED(client);
  struct zwnr_gl_vertex_array_impl *self = wl_resource_get_user_data(resource);
  struct zwnr_gl_vertex_attrib *attrib =
      zwnr_gl_vertex_array_ensure_vertex_attrib(self, index);
  attrib->enabled = true;
  attrib->enable_changed = true;
}

static void
zwnr_gl_vertex_array_protocol_disable_vertex_attrib_array(
    struct wl_client *client, struct wl_resource *resource, uint32_t index)
{
  UNUSED(client);
  struct zwnr_gl_vertex_array_impl *self = wl_resource_get_user_data(resource);
  struct zwnr_gl_vertex_attrib *attrib =
      zwnr_gl_vertex_array_ensure_vertex_attrib(self, index);
  attrib->enabled = false;
  attrib->enable_changed = true;
}

static void
zwnr_gl_vertex_array_protocol_vertex_attrib_pointer(struct wl_client *client,
    struct wl_resource *resource, uint32_t index, int32_t size, uint32_t type,
    uint32_t normalized, int32_t stride, struct wl_array *offset_wl_array,
    struct wl_resource *gl_buffer)
{
  UNUSED(client);

  uint64_t offset;
  if (zn_array_to_uint64_t(offset_wl_array, &offset) != 0) {
    wl_resource_post_error(resource, ZWN_COMPOSITOR_ERROR_WL_ARRAY_SIZE,
        "offset is expected to be uint64 (%ld byte), but got %ld byte",
        sizeof(uint64_t), offset_wl_array->size);
    return;
  }

  struct zwnr_gl_vertex_array_impl *self = wl_resource_get_user_data(resource);
  struct zwnr_gl_vertex_attrib *attrib =
      zwnr_gl_vertex_array_ensure_vertex_attrib(self, index);

  attrib->size = size;
  attrib->type = type;
  attrib->normalized = normalized;
  attrib->stride = stride;
  attrib->offset = offset;
  attrib->gl_buffer_changed = true;
  zn_weak_resource_link(&attrib->gl_buffer, gl_buffer);
}

static const struct zwn_gl_vertex_array_interface implementation = {
    .destroy = zwnr_gl_vertex_array_protocol_destroy,
    .enable_vertex_attrib_array =
        zwnr_gl_vertex_array_protocol_enable_vertex_attrib_array,
    .disable_vertex_attrib_array =
        zwnr_gl_vertex_array_protocol_disable_vertex_attrib_array,
    .vertex_attrib_pointer =
        zwnr_gl_vertex_array_protocol_vertex_attrib_pointer,
};

void
zwnr_gl_vertex_array_commit(struct zwnr_gl_vertex_array_impl *self)
{
  struct zwnr_gl_vertex_attrib *attrib, *tmp;

  wl_list_for_each_safe (
      attrib, tmp, &self->base.current.vertex_attrib_list, link) {
    zwnr_gl_vertex_attrib_destroy(attrib);
  }

  wl_list_for_each (attrib, &self->pending.vertex_attrib_list, link) {
    struct zwnr_gl_vertex_attrib *copy;
    struct zwnr_gl_buffer_impl *buffer;

    copy = zwnr_gl_vertex_attrib_copy(attrib);
    wl_list_insert(&self->base.current.vertex_attrib_list, &copy->link);

    attrib->gl_buffer_changed = false;
    attrib->enable_changed = false;

    buffer = zn_weak_resource_get_user_data(&attrib->gl_buffer);
    if (buffer) {
      zwnr_gl_buffer_commit(buffer);
    }
  }
}

struct zwnr_gl_vertex_array_impl *
zwnr_gl_vertex_array_create(struct wl_client *client, uint32_t id)
{
  struct zwnr_gl_vertex_array_impl *self;
  struct wl_resource *resource;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_signal_init(&self->base.events.destroy);
  wl_list_init(&self->base.current.vertex_attrib_list);
  wl_list_init(&self->pending.vertex_attrib_list);

  resource = wl_resource_create(client, &zwn_gl_vertex_array_interface, 1, id);
  if (resource == NULL) {
    zn_error("Failed to create a wl_resource");
    goto err_free;
  }

  wl_resource_set_implementation(
      resource, &implementation, self, zwnr_gl_vertex_array_handle_destroy);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zwnr_gl_vertex_array_destroy(struct zwnr_gl_vertex_array_impl *self)
{
  wl_signal_emit(&self->base.events.destroy, NULL);
  struct zwnr_gl_vertex_attrib *attrib, *tmp;

  wl_list_for_each_safe (attrib, tmp, &self->pending.vertex_attrib_list, link) {
    zwnr_gl_vertex_attrib_destroy(attrib);
  }

  wl_list_for_each_safe (
      attrib, tmp, &self->base.current.vertex_attrib_list, link) {
    zwnr_gl_vertex_attrib_destroy(attrib);
  }

  wl_list_remove(&self->base.events.destroy.listener_list);
  free(self);
}
