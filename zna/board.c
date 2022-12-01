#include "board.h"

#include <GLES3/gl32.h>
#include <cglm/affine.h>
#include <cglm/mat4.h>
#include <cglm/quat.h>
#include <zen-common.h>

#include "system.h"

/**
 * @param session must not be null
 */
static void
zna_board_setup_renderer_object(
    struct zna_board *self, struct znr_session *session)
{
  self->virtual_object = znr_virtual_object_create(session);
  zna_base_unit_setup_renderer_objects(
      self->base_unit, session, self->virtual_object);

  float scale_x = self->zn_board->geometry.size[0];
  float scale_y = self->zn_board->geometry.size[1];

  mat4 rotate;
  glm_quat_mat4(self->zn_board->geometry.quaternion, rotate);

  mat4 local_model = GLM_MAT4_IDENTITY_INIT;
  glm_translate(local_model, self->zn_board->geometry.center);
  glm_mat4_mul(local_model, rotate, local_model);
  glm_scale(local_model, (vec3){scale_x, scale_y, 1});

  znr_gl_base_technique_gl_uniform_matrix(self->base_unit->technique, 0,
      "local_model", 4, 4, 1, false, local_model[0]);
  znr_gl_base_technique_gl_uniform_vector(self->base_unit->technique, 0,
      "color", ZGN_GL_BASE_TECHNIQUE_UNIFORM_VARIABLE_TYPE_FLOAT, 3, 1,
      self->zn_board->color);

  znr_virtual_object_commit(self->virtual_object);
}

static void
zna_board_teardown_renderer_object(struct zna_board *self)
{
  zna_base_unit_teardown_renderer_objects(self->base_unit);

  if (self->virtual_object) {
    znr_virtual_object_destroy(self->virtual_object);
    self->virtual_object = NULL;
  }
}

static void
zna_board_handle_session_created(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zna_board *self =
      zn_container_of(listener, self, session_created_listener);
  struct znr_session *session = self->system->current_session;

  zna_board_setup_renderer_object(self, session);
}

static void
zna_board_handle_session_destroyed(struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zna_board *self =
      zn_container_of(listener, self, session_destroyed_listener);

  zna_board_teardown_renderer_object(self);
}

struct zna_board *
zna_board_create(struct zn_board *board, struct zna_system *system)
{
  struct zna_board *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zn_board = board;
  self->system = system;
  self->virtual_object = NULL;

  self->session_created_listener.notify = zna_board_handle_session_created;
  wl_signal_add(
      &system->events.current_session_created, &self->session_created_listener);

  self->session_destroyed_listener.notify = zna_board_handle_session_destroyed;
  wl_signal_add(&system->events.current_session_destroyed,
      &self->session_destroyed_listener);

  {  // setup base unit
    float vertices[4][2] = {
        {-0.5, -0.5},
        {+0.5, -0.5},
        {+0.5, +0.5},
        {-0.5, +0.5},
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
    vertex_attribute->stride = 0;
    vertex_attribute->offset = 0;

    union zgnr_gl_base_technique_draw_args draw_args;
    draw_args.arrays.mode = GL_TRIANGLE_FAN;
    draw_args.arrays.first = 0;
    draw_args.arrays.count = 4;

    self->base_unit = zna_base_unit_create(system, ZNA_SHADER_BOARD_VERTEX,
        ZNA_SHADER_BOARD_FRAGMENT, vertex_buffer, &vertex_attributes,
        ZGNR_GL_BASE_TECHNIQUE_DRAW_METHOD_ARRAYS, draw_args);

    wl_array_release(&vertex_attributes);

    zgnr_mem_storage_unref(vertex_buffer);
  }

  struct znr_session *session = self->system->current_session;
  if (session) {
    zna_board_setup_renderer_object(self, session);
  }

  return self;

err:
  return NULL;
}

void
zna_board_destroy(struct zna_board *self)
{
  if (self->virtual_object) znr_virtual_object_destroy(self->virtual_object);
  zna_base_unit_destroy(self->base_unit);
  wl_list_remove(&self->session_created_listener.link);
  wl_list_remove(&self->session_destroyed_listener.link);
  free(self);
}
