#include "gl-base-technique.h"

#include <zen-common.h>
#include <zgnr/gl-uniform-variable.h>
#include <zgnr/texture-binding.h>

#include "virtual-object/gl-buffer.h"
#include "virtual-object/gl-program.h"
#include "virtual-object/gl-texture.h"
#include "virtual-object/gl-vertex-array.h"
#include "virtual-object/rendering-unit.h"

static void zna_gl_base_technique_destroy(struct zna_gl_base_technique *self);

static struct zna_rendering_unit *
zna_gl_base_technique_get_rendering_unit(struct zna_gl_base_technique *self)
{
  struct zna_rendering_unit *unit =
      self->zgnr_gl_base_technique->unit->user_data;

  return unit;
}

static void
zna_gl_base_technique_handle_zgnr_gl_base_technique_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zna_gl_base_technique *self =
      zn_container_of(listener, self, zgnr_gl_base_technique_destroy_listener);

  zna_gl_base_technique_destroy(self);
}

static void
zna_gl_base_technique_handle_session_destroyed(
    struct wl_listener *listener, void *data)
{
  struct zna_gl_base_technique *self =
      zn_container_of(listener, self, session_destroyed_listener);
  UNUSED(data);

  if (self->znr_gl_base_technique) {
    znr_gl_base_technique_destroy(self->znr_gl_base_technique);
    self->znr_gl_base_technique = NULL;
  }
}

void
zna_gl_base_technique_apply_commit(
    struct zna_gl_base_technique *self, bool only_damaged)
{
  struct znr_session *session = self->system->current_session;
  struct zgnr_gl_uniform_variable *uniform_variable;
  struct zna_rendering_unit *unit =
      zna_gl_base_technique_get_rendering_unit(self);

  if (self->znr_gl_base_technique == NULL) {
    self->znr_gl_base_technique =
        znr_gl_base_technique_create(session, unit->znr_rendering_unit);
  }

  if (self->zgnr_gl_base_technique->current.vertex_array) {
    struct zna_gl_vertex_array *vertex_array =
        self->zgnr_gl_base_technique->current.vertex_array->user_data;

    zna_gl_vertex_array_apply_commit(vertex_array, only_damaged);

    if (self->zgnr_gl_base_technique->current.vertex_array_changed ||
        !only_damaged) {
      znr_gl_base_technique_bind_vertex_array(
          self->znr_gl_base_technique, vertex_array->znr_gl_vertex_array);
    }
  }

  if (self->zgnr_gl_base_technique->current.program) {
    struct zna_gl_program *program =
        self->zgnr_gl_base_technique->current.program->user_data;

    zna_gl_program_apply_commit(program, only_damaged);

    if (self->zgnr_gl_base_technique->current.program_changed ||
        !only_damaged) {
      znr_gl_base_technique_bind_program(
          self->znr_gl_base_technique, program->znr_gl_program);
    }
  }

  struct zgnr_texture_binding *texture_binding;
  wl_list_for_each (texture_binding,
      &self->zgnr_gl_base_technique->current.texture_binding_list, link) {
    struct zna_gl_texture *texture = texture_binding->texture->user_data;
    zna_gl_texture_apply_commit(texture, only_damaged);

    if (self->zgnr_gl_base_technique->current.texture_changed ||
        !only_damaged) {
      znr_gl_base_technique_bind_texture(self->znr_gl_base_technique,
          texture_binding->binding, texture_binding->name,
          texture->znr_gl_texture, texture_binding->target);
    }
  }

  wl_list_for_each (uniform_variable,
      &self->zgnr_gl_base_technique->current.uniform_variable_list, link) {
    if (uniform_variable->newly_comitted || !only_damaged) {
      if (uniform_variable->col == 1) {
        znr_gl_base_technique_gl_uniform_vector(self->znr_gl_base_technique,
            uniform_variable->location, uniform_variable->name,
            uniform_variable->type, uniform_variable->row,
            uniform_variable->count, uniform_variable->value);
      } else {
        znr_gl_base_technique_gl_uniform_matrix(self->znr_gl_base_technique,
            uniform_variable->location, uniform_variable->name,
            uniform_variable->col, uniform_variable->row,
            uniform_variable->count, uniform_variable->transpose,
            uniform_variable->value);
      }
    }
  }

  switch (self->zgnr_gl_base_technique->current.draw_method) {
    case ZGNR_GL_BASE_TECHNIQUE_DRAW_METHOD_NONE:
      break;
    case ZGNR_GL_BASE_TECHNIQUE_DRAW_METHOD_ARRAYS:
      if (self->zgnr_gl_base_technique->current.draw_method_changed ||
          !only_damaged) {
        znr_gl_base_technique_draw_arrays(self->znr_gl_base_technique,
            self->zgnr_gl_base_technique->current.args.arrays.mode,
            self->zgnr_gl_base_technique->current.args.arrays.first,
            self->zgnr_gl_base_technique->current.args.arrays.count);
      }
      break;
    case ZGNR_GL_BASE_TECHNIQUE_DRAW_METHOD_ELEMENTS:
      if (self->zgnr_gl_base_technique->current.element_array_buffer) {
        struct zna_gl_buffer *element_array_buffer =
            self->zgnr_gl_base_technique->current.element_array_buffer
                ->user_data;

        zna_gl_buffer_apply_commit(element_array_buffer, only_damaged);

        if (self->zgnr_gl_base_technique->current.draw_method_changed ||
            !only_damaged) {
          znr_gl_base_technique_draw_elements(self->znr_gl_base_technique,
              self->zgnr_gl_base_technique->current.args.elements.mode,
              self->zgnr_gl_base_technique->current.args.elements.count,
              self->zgnr_gl_base_technique->current.args.elements.type,
              self->zgnr_gl_base_technique->current.args.elements.offset,
              element_array_buffer->znr_gl_buffer);
        }
      }
      break;
  }
}

struct zna_gl_base_technique *
zna_gl_base_technique_create(
    struct zgnr_gl_base_technique *zgnr_gl_base_technique,
    struct zna_system *system)
{
  struct zna_gl_base_technique *self;

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
zna_gl_base_technique_destroy(struct zna_gl_base_technique *self)
{
  if (self->znr_gl_base_technique)
    znr_gl_base_technique_destroy(self->znr_gl_base_technique);
  wl_list_remove(&self->zgnr_gl_base_technique_destroy_listener.link);
  wl_list_remove(&self->session_destroyed_listener.link);
  free(self);
}
