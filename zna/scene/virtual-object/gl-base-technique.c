#include "gl-base-technique.h"

#include <zen-common.h>

#include "scene/virtual-object/gl-buffer.h"

static void zna_gl_base_technique_destroy(struct zna_gl_base_technique* self);

static void
zna_gl_base_technique_handle_zgnr_gl_base_technique_destroy(
    struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zna_gl_base_technique* self =
      zn_container_of(listener, self, zgnr_gl_base_technique_destroy_listener);

  zna_gl_base_technique_destroy(self);
}

static void
zna_gl_base_technique_handle_session_destroyed(
    struct wl_listener* listener, void* data)
{
  struct zna_gl_base_technique* self =
      zn_container_of(listener, self, session_destroyed_listener);
  UNUSED(self);
  UNUSED(data);
}

void
zna_gl_base_technique_apply_commit(
    struct zna_gl_base_technique* self, bool only_damaged)
{
  struct zgnr_gl_vertex_array* vertex_array;

  vertex_array = self->zgnr_gl_base_technique->current.vertex_array;
  if (vertex_array) {
    struct zgnr_gl_vertex_attrib* vertex_attrib;
    wl_array_for_each (vertex_attrib, &vertex_array->current.vertex_attribs) {
      struct zgnr_gl_buffer* zgnr_gl_buffer =
          zgnr_gl_vertex_attrib_get_gl_buffer(vertex_attrib);
      if (zgnr_gl_buffer == NULL) continue;

      struct zna_gl_buffer* gl_buffer = zgnr_gl_buffer->user_data;
      zna_gl_buffer_apply_commit(gl_buffer, only_damaged);
    }
  }

  // TODO: Apply draw arrays, etc.
}

struct zna_gl_base_technique*
zna_gl_base_technique_create(
    struct zgnr_gl_base_technique* zgnr_gl_base_technique,
    struct zna_system* system)
{
  struct zna_gl_base_technique* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zgnr_gl_base_technique = zgnr_gl_base_technique;
  zgnr_gl_base_technique->user_data = self;
  self->system = system;

  self->zgnr_gl_base_technique_destroy_listener.notify =
      zna_gl_base_technique_handle_zgnr_gl_base_technique_destroy;
  wl_signal_add(&self->zgnr_gl_base_technique->events.destroy,
      &self->zgnr_gl_base_technique_destroy_listener);

  self->session_destroyed_listener.notify =
      zna_gl_base_technique_handle_session_destroyed;
  wl_signal_add(&self->system->events.current_session_destroyed,
      &self->session_destroyed_listener);

  return self;

err:
  return NULL;
}

static void
zna_gl_base_technique_destroy(struct zna_gl_base_technique* self)
{
  wl_list_remove(&self->zgnr_gl_base_technique_destroy_listener.link);
  wl_list_remove(&self->session_destroyed_listener.link);
  free(self);
}
