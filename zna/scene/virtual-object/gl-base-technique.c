#include "gl-base-technique.h"

#include <zen-common.h>

#include "scene/virtual-object/gl-vertex-array.h"
#include "scene/virtual-object/rendering-unit.h"

static void zna_gl_base_technique_destroy(struct zna_gl_base_technique* self);

static struct zna_rendering_unit*
zna_gl_base_technique_get_rendering_unit(struct zna_gl_base_technique* self)
{
  struct zna_rendering_unit* unit =
      self->zgnr_gl_base_technique->unit->user_data;

  return unit;
}

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
  UNUSED(data);

  if (self->znr_gl_base_technique) {
    znr_gl_base_technique_destroy(self->znr_gl_base_technique);
    self->znr_gl_base_technique = NULL;
  }
}

void
zna_gl_base_technique_apply_commit(
    struct zna_gl_base_technique* self, bool only_damaged)
{
  struct znr_session* session = self->system->current_session;
  struct zna_rendering_unit* unit =
      zna_gl_base_technique_get_rendering_unit(self);
  if (self->znr_gl_base_technique == NULL) {
    self->znr_gl_base_technique =
        znr_gl_base_technique_create(session, unit->znr_rendering_unit);
  }

  if (self->zgnr_gl_base_technique->current.vertex_array) {
    struct zna_gl_vertex_array* vertex_array =
        self->zgnr_gl_base_technique->current.vertex_array->user_data;

    zna_gl_vertex_array_apply_commit(vertex_array, only_damaged);

    if (self->zgnr_gl_base_technique->current.vertex_array_changed ||
        !only_damaged) {
      znr_gl_base_technique_bind_vertex_array(
          self->znr_gl_base_technique, vertex_array->znr_gl_vertex_array);
    }
  }

  switch (self->zgnr_gl_base_technique->current.draw_method) {
    case ZGNR_GL_BASE_TECHNIQUE_DRAW_ARRAYS:
      znr_gl_base_technique_draw_arrays(self->znr_gl_base_technique,
          self->zgnr_gl_base_technique->current.args.arrays.mode,
          self->zgnr_gl_base_technique->current.args.arrays.first,
          self->zgnr_gl_base_technique->current.args.arrays.count);
      break;
  }
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
  if (self->znr_gl_base_technique)
    znr_gl_base_technique_destroy(self->znr_gl_base_technique);
  wl_list_remove(&self->zgnr_gl_base_technique_destroy_listener.link);
  wl_list_remove(&self->session_destroyed_listener.link);
  free(self);
}
