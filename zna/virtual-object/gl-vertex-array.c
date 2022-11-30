#include "gl-vertex-array.h"

#include <zen-common.h>

#include "virtual-object/gl-buffer.h"

static void zna_gl_vertex_array_destroy(struct zna_gl_vertex_array* self);

static void
zna_gl_vertex_array_handle_session_destroy(
    struct wl_listener* listener, void* data)
{
  struct zna_gl_vertex_array* self =
      zn_container_of(listener, self, session_destroy_listener);
  UNUSED(data);

  if (self->znr_gl_vertex_array) {
    znr_gl_vertex_array_destroy(self->znr_gl_vertex_array);
    self->znr_gl_vertex_array = NULL;
  }
}

static void
zna_gl_vertex_array_handle_zgnr_gl_vertex_array_destroy(
    struct wl_listener* listener, void* data)
{
  struct zna_gl_vertex_array* self =
      zn_container_of(listener, self, zgnr_gl_vertex_array_destroy_listener);
  UNUSED(data);

  zna_gl_vertex_array_destroy(self);
}

void
zna_gl_vertex_array_apply_commit(
    struct zna_gl_vertex_array* self, bool only_damage)
{
  struct znr_session* session = self->system->current_session;
  struct zgnr_gl_vertex_attrib* vertex_attrib;

  if (self->znr_gl_vertex_array == NULL) {
    self->znr_gl_vertex_array = znr_gl_vertex_array_create(session);
  }

  wl_list_for_each (vertex_attrib,
      &self->zgnr_gl_vertex_array->current.vertex_attrib_list, link) {
    struct zgnr_gl_buffer* zgnr_gl_buffer =
        zgnr_gl_vertex_attrib_get_gl_buffer(vertex_attrib);
    if (zgnr_gl_buffer == NULL) continue;

    struct zna_gl_buffer* gl_buffer = zgnr_gl_buffer->user_data;
    zna_gl_buffer_apply_commit(gl_buffer, only_damage);

    if (vertex_attrib->enable_changed || !only_damage) {
      if (vertex_attrib->enabled) {
        znr_gl_vertex_array_enable_vertex_attrib_array(
            self->znr_gl_vertex_array, vertex_attrib->index);
      } else {
        znr_gl_vertex_array_disable_vertex_attrib_array(
            self->znr_gl_vertex_array, vertex_attrib->index);
      }
    }

    if (vertex_attrib->gl_buffer_changed || !only_damage) {
      znr_gl_vertex_array_vertex_attrib_pointer(self->znr_gl_vertex_array,
          vertex_attrib->index, vertex_attrib->size, vertex_attrib->type,
          vertex_attrib->normalized, vertex_attrib->stride,
          vertex_attrib->offset, gl_buffer->znr_gl_buffer);
    }
  }
}

struct zna_gl_vertex_array*
zna_gl_vertex_array_create(struct zgnr_gl_vertex_array* zgnr_gl_vertex_array,
    struct zna_system* system)
{
  struct zna_gl_vertex_array* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zgnr_gl_vertex_array = zgnr_gl_vertex_array;
  zgnr_gl_vertex_array->user_data = self;
  self->system = system;
  self->znr_gl_vertex_array = NULL;

  self->zgnr_gl_vertex_array_destroy_listener.notify =
      zna_gl_vertex_array_handle_zgnr_gl_vertex_array_destroy;
  wl_signal_add(&self->zgnr_gl_vertex_array->events.destroy,
      &self->zgnr_gl_vertex_array_destroy_listener);

  self->session_destroy_listener.notify =
      zna_gl_vertex_array_handle_session_destroy;
  wl_signal_add(&self->system->events.current_session_destroyed,
      &self->session_destroy_listener);

  return self;

err:
  return NULL;
}

static void
zna_gl_vertex_array_destroy(struct zna_gl_vertex_array* self)
{
  if (self->znr_gl_vertex_array)
    znr_gl_vertex_array_destroy(self->znr_gl_vertex_array);
  wl_list_remove(&self->session_destroy_listener.link);
  wl_list_remove(&self->zgnr_gl_vertex_array_destroy_listener.link);
  free(self);
}
