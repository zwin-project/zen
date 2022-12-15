#include "view.h"

#include <GLES3/gl32.h>
#include <cglm/affine.h>
#include <cglm/mat4.h>
#include <zen-common.h>

#include "zen/view.h"

void
zna_view_commit(struct zna_view *self, uint32_t damage)
{
  if (!self->base_unit->has_renderer_objects) return;

  if (damage & ZNA_VIEW_DAMAGE_GEOMETRY) {
    mat4 local_model = GLM_MAT4_IDENTITY_INIT;

    glm_mat4_copy(self->zn_view->geometry.transform, local_model);
    glm_scale(local_model, (vec3){self->zn_view->geometry.size[0],
                               self->zn_view->geometry.size[1], 0});

    znr_gl_base_technique_gl_uniform_matrix(self->base_unit->technique, 0,
        "local_model", 4, 4, 1, false, local_model[0]);
  }

  if (damage & ZNA_VIEW_DAMAGE_TEXTURE) {
    struct wlr_texture *texture =
        wlr_surface_get_texture(self->zn_view->surface);
    if (texture) {
      zna_base_unit_read_wlr_texture(self->base_unit, texture);
      znr_gl_sampler_parameter_i(
          self->base_unit->sampler0, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      znr_gl_sampler_parameter_i(
          self->base_unit->sampler0, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
  }

  znr_virtual_object_commit(self->virtual_object);
}

/**
 * @param session must not be null
 */
static void
zna_view_setup_renderer_objects(
    struct zna_view *self, struct znr_session *session)
{
  self->virtual_object = znr_virtual_object_create(session);

  zna_base_unit_setup_renderer_objects(
      self->base_unit, session, self->virtual_object);

  zna_view_commit(self, UINT32_MAX);
}

static void
zna_view_teardown_renderer_objects(struct zna_view *self)
{
  zna_base_unit_teardown_renderer_objects(self->base_unit);

  if (self->virtual_object) {
    znr_virtual_object_destroy(self->virtual_object);
    self->virtual_object = NULL;
  }
}

static void
zna_view_handle_session_created(struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zna_view *self =
      zn_container_of(listener, self, session_created_listener);
  struct znr_session *session = self->system->current_session;

  zna_view_setup_renderer_objects(self, session);
}

static void
zna_view_handle_session_destroyed(struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zna_view *self =
      zn_container_of(listener, self, session_destroyed_listener);

  zna_view_teardown_renderer_objects(self);
}

struct zna_view *
zna_view_create(struct zn_view *zn_view, struct zna_system *system)
{
  struct zna_view *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->session_created_listener.notify = zna_view_handle_session_created;
  wl_signal_add(
      &system->events.current_session_created, &self->session_created_listener);

  self->session_destroyed_listener.notify = zna_view_handle_session_destroyed;
  wl_signal_add(&system->events.current_session_destroyed,
      &self->session_destroyed_listener);

  self->zn_view = zn_view;
  self->system = system;
  self->virtual_object = NULL;

  {  // setup base unit
    float vertices[4][4] = {
        {0, -1, 0, 1},  // {x, y, u, v}
        {1, -1, 1, 1},
        {1, +0, 1, 0},
        {0, +0, 0, 0},
    };

    struct zgnr_mem_storage *vertex_buffer =
        zgnr_mem_storage_create(vertices, sizeof(vertices));

    struct wl_array vertex_attributes;
    wl_array_init(&vertex_attributes);

    struct zna_base_unit_vertex_attribute *vertex_attribute =
        wl_array_add(&vertex_attributes, sizeof *vertex_attribute);
    vertex_attribute->index = 0;
    vertex_attribute->size = 2;
    vertex_attribute->type = GL_FLOAT;
    vertex_attribute->normalized = GL_FALSE;
    vertex_attribute->stride = sizeof(vertices[0]);
    vertex_attribute->offset = 0;

    vertex_attribute =
        wl_array_add(&vertex_attributes, sizeof *vertex_attribute);
    vertex_attribute->index = 1;
    vertex_attribute->size = 2;
    vertex_attribute->type = GL_FLOAT;
    vertex_attribute->normalized = GL_FALSE;
    vertex_attribute->stride = sizeof(vertices[0]);
    vertex_attribute->offset = sizeof(float) * 2;

    union zgnr_gl_base_technique_draw_args draw_args;
    draw_args.arrays.mode = GL_TRIANGLE_FAN;
    draw_args.arrays.first = 0;
    draw_args.arrays.count = 4;

    self->base_unit = zna_base_unit_create(system, ZNA_SHADER_VIEW_VERTEX,
        ZNA_SHADER_VIEW_FRAGMENT, vertex_buffer, &vertex_attributes,
        ZGNR_GL_BASE_TECHNIQUE_DRAW_METHOD_ARRAYS, draw_args);

    wl_array_release(&vertex_attributes);

    zgnr_mem_storage_unref(vertex_buffer);
  }

  struct znr_session *session = self->system->current_session;
  if (session) {
    zna_view_setup_renderer_objects(self, session);
  }

  return self;

err:
  return NULL;
}

void
zna_view_destroy(struct zna_view *self)
{
  if (self->virtual_object) znr_virtual_object_destroy(self->virtual_object);
  zna_base_unit_destroy(self->base_unit);
  wl_list_remove(&self->session_created_listener.link);
  wl_list_remove(&self->session_destroyed_listener.link);
  free(self);
}
