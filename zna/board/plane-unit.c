#include "plane-unit.h"

#include <GLES3/gl32.h>
#include <cglm/affine.h>
#include <cglm/mat4.h>
#include <zen-common.h>

void
zna_board_plane_unit_commit(struct zna_board_plane_unit *self,
    struct zn_board *board, struct znr_virtual_object *virtual_object)
{
  if (!self->base_unit->has_renderer_objects) return;

  float scale_x = board->geometry.size[0];
  float scale_y = board->geometry.size[1];
  double width, height;
  vec2 effective_resolution;

  mat4 local_model = GLM_MAT4_IDENTITY_INIT;

  glm_mat4_copy(board->geometry.transform, local_model);
  glm_scale(local_model, (vec3){scale_x, scale_y, 1});
  zn_board_get_effective_size(board, &width, &height);
  effective_resolution[0] = width;
  effective_resolution[1] = height;

  znr_gl_base_technique_gl_uniform_vector(self->base_unit->technique, 0,
      "effective_resolution", ZWN_GL_BASE_TECHNIQUE_UNIFORM_VARIABLE_TYPE_FLOAT,
      2, 1, effective_resolution);
  znr_gl_base_technique_gl_uniform_matrix(self->base_unit->technique, 0,
      "local_model", 4, 4, 1, false, local_model[0]);

  znr_virtual_object_commit(virtual_object);
}

void
zna_board_plane_unit_setup_renderer_objects(struct zna_board_plane_unit *self,
    struct znr_dispatcher *dispatcher,
    struct znr_virtual_object *virtual_object)
{
  zna_base_unit_setup_renderer_objects(
      self->base_unit, dispatcher, virtual_object);
}

void
zna_board_plane_unit_teardown_renderer_objects(
    struct zna_board_plane_unit *self)
{
  zna_base_unit_teardown_renderer_objects(self->base_unit);
}

struct zna_board_plane_unit *
zna_board_plane_unit_create(struct zna_system *system)
{
  struct zna_board_plane_unit *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  float vertices[4][2] = {
      {-0.5, 0.f},
      {+0.5, 0.f},
      {+0.5, 1.f},
      {-0.5, 1.f},
  };

  struct zwnr_mem_storage *vertex_buffer =
      zwnr_mem_storage_create(vertices, sizeof(vertices));

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

  union zwnr_gl_base_technique_draw_args draw_args;
  draw_args.arrays.mode = GL_TRIANGLE_FAN;
  draw_args.arrays.first = 0;
  draw_args.arrays.count = 4;

  self->base_unit = zna_base_unit_create(system, ZNA_SHADER_BOARD_VERTEX,
      ZNA_SHADER_BOARD_FRAGMENT, vertex_buffer, &vertex_attributes, NULL,
      ZWNR_GL_BASE_TECHNIQUE_DRAW_METHOD_ARRAYS, draw_args);

  wl_array_release(&vertex_attributes);

  zwnr_mem_storage_unref(vertex_buffer);

  return self;

err:
  return NULL;
}

void
zna_board_plane_unit_destroy(struct zna_board_plane_unit *self)
{
  zna_base_unit_destroy(self->base_unit);
  free(self);
}
