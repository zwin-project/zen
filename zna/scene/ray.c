#include "ray.h"

#include <GLES3/gl32.h>
#include <zen-common.h>

#include "system.h"

static void
zna_ray_handle_motion(struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zna_ray* self = zn_container_of(listener, self, motion_listener);

  if (self->base_unit->has_renderer_objects) {
    vec3 tip;
    zn_ray_get_tip(self->zn_ray, 1.0, tip);
    znr_gl_base_technique_gl_uniform_vector(self->base_unit->technique, 0,
        "tip", ZGN_GL_BASE_TECHNIQUE_UNIFORM_VARIABLE_TYPE_FLOAT, 3, 1, tip);
    znr_virtual_object_commit(self->virtual_object);
  }
}

/**
 * @param session must not be null
 */
static void
zna_ray_setup_renderer_objects(
    struct zna_ray* self, struct znr_session* session)
{
  self->virtual_object = znr_virtual_object_create(session);

  zna_base_unit_setup_renderer_objects(
      self->base_unit, session, self->virtual_object);

  vec3 tip;
  zn_ray_get_tip(self->zn_ray, 1.0, tip);
  znr_gl_base_technique_gl_uniform_vector(self->base_unit->technique, 0, "tip",
      ZGN_GL_BASE_TECHNIQUE_UNIFORM_VARIABLE_TYPE_FLOAT, 3, 1, tip);

  znr_gl_base_technique_gl_uniform_vector(self->base_unit->technique, 0,
      "origin", ZGN_GL_BASE_TECHNIQUE_UNIFORM_VARIABLE_TYPE_FLOAT, 3, 1,
      self->zn_ray->origin);

  znr_virtual_object_commit(self->virtual_object);
}

static void
zna_ray_teardown_renderer_objects(struct zna_ray* self)
{
  zna_base_unit_teardown_renderer_objects(self->base_unit);

  if (self->virtual_object) {
    znr_virtual_object_destroy(self->virtual_object);
    self->virtual_object = NULL;
  }
}

static void
zna_ray_handle_session_created(struct wl_listener* listener, void* data)
{
  UNUSED(data);

  struct zna_ray* self =
      zn_container_of(listener, self, session_created_listener);
  struct znr_session* session = self->system->current_session;

  zna_ray_setup_renderer_objects(self, session);
}

static void
zna_ray_handle_session_destroyed(struct wl_listener* listener, void* data)
{
  UNUSED(data);

  struct zna_ray* self =
      zn_container_of(listener, self, session_destroyed_listener);

  zna_ray_teardown_renderer_objects(self);
}

struct zna_ray*
zna_ray_create(struct zn_ray* ray, struct zna_system* system)
{
  struct zna_ray* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->motion_listener.notify = zna_ray_handle_motion;
  wl_signal_add(&ray->events.motion, &self->motion_listener);

  self->session_created_listener.notify = zna_ray_handle_session_created;
  wl_signal_add(
      &system->events.current_session_created, &self->session_created_listener);

  self->session_destroyed_listener.notify = zna_ray_handle_session_destroyed;
  wl_signal_add(&system->events.current_session_destroyed,
      &self->session_destroyed_listener);

  self->zn_ray = ray;
  self->system = system;
  self->virtual_object = NULL;

  {  // setup base unit
    float vertices[2] = {0, 1};
    struct zgnr_mem_storage* vertex_buffer =
        zgnr_mem_storage_create(vertices, sizeof(vertices));

    struct wl_array vertex_attributes;
    wl_array_init(&vertex_attributes);

    struct zna_base_unit_vertex_attribute* vertex_attribute =
        wl_array_add(&vertex_attributes, sizeof *vertex_attribute);
    vertex_attribute->index = 0;
    vertex_attribute->size = 1;
    vertex_attribute->type = GL_FLOAT;
    vertex_attribute->normalized = GL_FALSE;
    vertex_attribute->stride = 0;
    vertex_attribute->offset = 0;

    union zgnr_gl_base_technique_draw_args draw_args;
    draw_args.arrays.mode = GL_LINES;
    draw_args.arrays.first = 0;
    draw_args.arrays.count = 2;

    self->base_unit = zna_base_unit_create(system, ZNA_SHADER_RAY_VERTEX,
        ZNA_SHADER_COLOR_FRAGMENT, vertex_buffer, &vertex_attributes,
        ZGNR_GL_BASE_TECHNIQUE_DRAW_METHOD_ARRAYS, draw_args);

    wl_array_release(&vertex_attributes);

    zgnr_mem_storage_unref(vertex_buffer);
  }

  struct znr_session* session = self->system->current_session;
  if (session) {
    zna_ray_setup_renderer_objects(self, session);
  }

  return self;

err:
  return NULL;
}

void
zna_ray_destroy(struct zna_ray* self)
{
  if (self->virtual_object) znr_virtual_object_destroy(self->virtual_object);
  zna_base_unit_destroy(self->base_unit);
  wl_list_remove(&self->session_created_listener.link);
  wl_list_remove(&self->session_destroyed_listener.link);
  wl_list_remove(&self->motion_listener.link);
  free(self);
}
